#include <QStringList>
#include <QMutex>
#include <QWaitCondition>
#include <plugininterface/tcpsocket.h>
#include <plugininterface/proxy.h>
#include <plugininterface/addressparser.h>
#include <plugininterface/socketexceptions.h>
#include <exceptions/notimplemented.h>
#include "httpimp.h"
#include "xmlsettingsmanager.h"

HttpImp::HttpImp (QObject *parent)
: ImpBase (parent)
, Socket_ (0)
, Stop_ (false)
, GetFileSize_ (false)
{
    AwaitFileInfoReaction_.first = new QWaitCondition;
    AwaitFileInfoReaction_.second = new QMutex;
}

HttpImp::~HttpImp ()
{
    delete Socket_;
    delete AwaitFileInfoReaction_.first;
    delete AwaitFileInfoReaction_.second;
}

void HttpImp::SetRestartPosition (length_t pos)
{
    RestartPosition_ = pos;
}

void HttpImp::SetURL (const QString& url)
{
    URL_ = url;
}

void HttpImp::StopDownload ()
{
    Stop_ = true;
}

void HttpImp::ReactedToFileInfo ()
{
    AwaitFileInfoReaction_.first->wakeAll ();
}

void HttpImp::ScheduleGetFileSize ()
{
    GetFileSize_ = true;
}

void HttpImp::run ()
{
    Socket_ = new TcpSocket;
    Socket_->SetURL (URL_);
    Socket_->SetDefaultTimeout (XmlSettingsManager::Instance ()->property ("ConnectTimeout").toInt ());

    try
    {
        Socket_->Connect (Socket_->GetAddressParser ()->GetHost (), Socket_->GetAddressParser ()->GetPort ());
    }
    catch (const Exceptions::Socket::BaseSocket& e)
    {
        emit error (tr ("Error while trying to connect to host %1, port %2: %3")
                .arg (Socket_->GetAddressParser ()->GetHost ())
                .arg (Socket_->GetAddressParser ()->GetPort ())
                .arg (e.GetReason ().c_str ()));
        emit stopped ();
        delete Socket_;
        Socket_ = 0;
        return;
    }
    Socket_->SetDefaultTimeout (XmlSettingsManager::Instance ()->property ("DefaultTimeout").toInt ());
    WriteHeaders ();
    msleep (100);
    bool abort = ReadResponse ();

    if (abort)
    {
        Socket_->Disconnect ();
        delete Socket_;
        Socket_ = 0;
        emit stopped ();
        return;
    }

    if (GetFileSize_)
    {
        delete Socket_;
        Socket_ = 0;
        emit stopped ();
        GetFileSize_ = false;
        return;
    }

    length_t counter = 0;

    int cacheSize = XmlSettingsManager::Instance ()->property ("CacheSize").toInt () * 1024; 
    SetCacheSize (cacheSize);
    Socket_->setReadBufferSize (cacheSize);

    AwaitFileInfoReaction_.second->lock ();
    AwaitFileInfoReaction_.first->wait (AwaitFileInfoReaction_.second);
    AwaitFileInfoReaction_.second->unlock ();

    emit dataFetched (RestartPosition_, Response_.ContentLength_ + RestartPosition_, QByteArray ());
    while (counter < Response_.ContentLength_)
    {
        QByteArray newData;
        msleep (10);
        try
        {
            newData = Socket_->ReadAll ();
        }
        catch (const Exceptions::Socket::SocketTimeout&)
        {
        }
        catch (const Exceptions::Socket::BaseSocket& e)
        {
            qDebug () << Q_FUNC_INFO << e.GetName ().c_str () << "\t\t" << e.GetReason ().c_str () << "\"";
            break;
        }
        catch (...)
        {
            qDebug () << Q_FUNC_INFO << "caught some strange exception";
            emit error (tr ("HTTP implementation failed in a very strange way. Please send to developers any .log files you find in application's directory and it's subdirectories. Thanks for your help."));
            break;
        }

        if (Stop_)
        {
            qDebug () << Q_FUNC_INFO << ": stopping.";
            Stop_ = false;
            break;
        }

        counter += newData.size ();
        Emit (counter + RestartPosition_, Response_.ContentLength_ + RestartPosition_, newData);
    }
    EmitFlush (counter + RestartPosition_, Response_.ContentLength_ + RestartPosition_);
    emit stopped ();

    if (counter == Response_.ContentLength_ || RequestedRangeNotSatisfiable == Response_.StatusCode_)
        emit finished ();

    delete Socket_;
    Socket_ = 0;
}

void HttpImp::WriteHeaders ()
{
    QString request = Socket_->GetAddressParser ()->GetPath ();
    QStringList splitted = URL_.split ('?');
    if (splitted.size () >= 2)
        request.append ('?').append (splitted [1]);
    Socket_->Write ("GET " + request + " HTTP/1.1\r\n");
    QString agent = XmlSettingsManager::Instance ()->property ("HTTPAgent").toString ();
    Socket_->Write ("User-Agent: " + agent.trimmed () + "\r\n");
    Socket_->Write (QString ("Accept: */*\r\n"));
    Socket_->Write ("Host: " + Socket_->GetAddressParser ()->GetHost () + "\r\n");
    if (RestartPosition_)
        Socket_->Write ("Range: bytes=" + QString::number (RestartPosition_) + "-\r\n");
    Socket_->Write (QString ("\r\n"));
    Socket_->Flush ();
}

bool HttpImp::ReadResponse ()
{
    QByteArray br;
    try
    {
        br = Socket_->ReadLine ();
    }
    catch (const Exceptions::Socket::SocketTimeout& e)
    {
        qDebug () << Q_FUNC_INFO << "Socket timeout";
        emit error (tr ("Failed to read response: socket timeout"));
        return true;
    }
    catch (const Exceptions::Socket::BaseSocket& e)
    {
        qDebug () << Q_FUNC_INFO << "caught \"" << e.GetName ().c_str () << "\", saying\"" << e.GetReason ().c_str () << "\"";
        return true;
    }
    ParseFirstLine (br);
    while (true)
    {
        QByteArray lineRead;
        try
        {
            lineRead = Socket_->ReadLine ();
        }
        catch (const Exceptions::Socket::BaseSocket& e)
        {
            qDebug () << Q_FUNC_INFO << "caught \"" << e.GetName ().c_str () << "\", saying\"" << e.GetReason ().c_str () << "\"";
            break;
        }
        if (lineRead.trimmed ().isEmpty ())
            break;
        ParseSingleLine (lineRead);
    }
    if (Response_.Fields_.contains ("content-length"))
    {
        Response_.ContentLength_ = Response_.Fields_ ["content-length"].toULongLong ();
        emit gotFileSize (Response_.ContentLength_);
    }

    bool shouldWeReturn = DoStuffWithResponse ();

    if (!shouldWeReturn)
    {
        QDateTime dt = QDateTime::fromString (Response_.Fields_ ["last-modified"].left (25), "ddd, dd MMM yyyy HH:mm:ss");
        RemoteFileInfo rfi = { true, dt, Response_.ContentLength_, Response_.Fields_ ["content-type"] };
        emit gotRemoteFileInfo (rfi);
    }

    return shouldWeReturn;
}

void HttpImp::ParseFirstLine (const QString& str)
{
    QStringList response = str.trimmed ().split (" ");
    QString protoVersion = response [0];
    int statusCode = response [1].toUInt ();
    QString rest;
    for (int i = 2; i < response.size (); ++i)
    {
        rest += response [i];
        rest += " ";
    }
    Response_.Proto_ = protoVersion;
    Response_.StatusCode_ = statusCode;
    Response_.StatusReason_ = rest;
}

bool HttpImp::DoStuffWithResponse ()
{
    switch (Response_.StatusCode_)
    {
        // All's ok
        case Continue:
        case SwitchingProtocols:
        case Processing_WEBDAV_:
        case OK:
        case Created:
        case Accepted:
        case NonAuthoritative:
        case NoContent:
        case ResetContent:
        case PartialContent:
        case MultiStatus_WEBDAV_:
            return false;

        // Will be handled later
        case MovedPermanently:
        case Found:
        case SeeOther:
        case TemporaryRedirect:
            DoRedirect ();
            return true;
        case RequestedRangeNotSatisfiable:
            emit finished ();
            return true;
        case BadRequest:
            emit error (tr ("400 Bad request. The request contains bad syntax or cannot be fulfilled."));
            return true;
        case Unauthorized:
            emit error (tr ("401 Unauthorized. Authentication is possible but has failed or not yet been provided."));
            return true;
        case Forbidden:
            emit error (tr ("403 Forbidden. The request was legal, but server is refusing to respond to it. Authenticating will make no difference."));
            return true;
        case NotFound:
            emit error (tr ("404 Resource not found."));
            return true;
        case MethodNotAllowed:
            emit error (tr ("405 Method not allowed. Request method not supported by the URL."));
            return true;
        case NotAcceptable:
            emit error (tr ("406 Not acceptable."));
            return true;
        case ProxyAuthenticationRequired:
            emit error (tr ("407 Proxy authentication required."));
            return true;
        case RequestTimeout:
            emit error (tr ("408 Request timeout."));
            return true;
        case Conflict:
            emit error (tr ("409 Conflict."));
            return true;
        case Gone:
            emit error (tr ("410 Gone. Resource is not available and will not be available again. Maybe it was intentionally removed."));
            return true;
        case LengthRequired:
            emit error (tr ("411 Length required."));
            return true;
        case PreconditionFailed:
            emit error (tr ("412 Precondition failed."));
            return true;
        case RequestEntityTooLarge:
            emit error (tr ("413 Request entity too large."));
            return true;
        case RequestURITooLong:
            emit error (tr ("414 Request URI too long."));
            return true;
        case UnsupportedMediaType:
            emit error (tr ("415 Unsupported media type."));
            return true;
        case ExpectationFailed:
            emit error (tr ("417 Expectation failed."));
            return true;
        case UnprocessableEntity_WEBDAV_:
            emit error (tr ("422 Unprocessable entity (WebDAV). The request was well-formed but was unable to be followed due to semantic errors."));
            return true;
        case Locked_WEBDAV_:
            emit error (tr ("423 Locked (WebDAV). The resource that is being accessed is locked."));
            return true;
        case FailedDependency_WEBDAV_:
            emit error (tr ("424 Failed dependency (WebDAV). The request failed due to failure of a previous request."));
            return true;
        case UnorderedCollection_WEBDAV_:
            emit error (tr ("425 Unordered collection (WebDAV). You really never should see this message."));
            return true;
        case UpgradeRequired:
            emit error (tr ("426 Upgrade required. The client should switch to TLS/1.0."));
            return true;
        case RetryWith:
            emit error (tr ("449 Retry with. A Microsoft extension: The request should be retried after doing the appropriate action."));
            return true;
        case InternalServerError:
            emit error (tr ("500 Internal server error. Server failed to fulfil the request due to misconfiguration."));
            return true;
        case NotImplemented:
            emit error (tr ("501 Not implemented."));
            return true;
        case BadGateway:
            emit error (tr ("502 Bad gateway."));
            return true;
        case ServiceUnavailable:
            emit error (tr ("503 Service unavailable."));
            return true;
        case GatewayTimeout:
            emit error (tr ("504 Gateway timeout."));
            return true;
        case HTTPVersionNotSupported:
            emit error (tr ("505 HTTP version not supported."));
            return true;
        case VariantAlsoNegotiates:
            emit error (tr ("506 Variant also negotiates."));
            return true;
        case InsufficientStorage_WEBDAV_:
            emit error (tr ("507 Insufficient storage (WebDAV)."));
            return true;
        case BandwidthLimitExceeded:
            emit error (tr ("509 Bandwidth limit exceeded."));
            return true;
        case NotExtented:
            emit error (tr ("510 Not extented."));
            return true;
        default:
            throw Exceptions::NotImplemented (QString ("The HTTP status code " + QString::number (Response_.StatusCode_) + " wasn't implemented yet. Please, send the bugreport to us with URL of file you've tried to download.").toStdString ());
    }
}

void HttpImp::ParseSingleLine (const QString& str)
{
    QString workingCopy = str.simplified ();
    QString key = workingCopy.section (": ", 0, 0).toLower ();
    QString value = workingCopy.section (": ", 1);
    Response_.Fields_ [key] = value;
}

void HttpImp::DoRedirect ()
{
    QString newLoc = Response_.Fields_ ["location"].trimmed ();
    emit clarifyURL (newLoc);
    disconnect (this, 0, 0, 0);
}

void HttpImp::Finalize ()
{
    Socket_->Disconnect ();
    delete Socket_;
    Socket_ = 0;
}

