#include <QStringList>
#include <QMutex>
#include <QWaitCondition>
#include <plugininterface/tcpsocket.h>
#include <plugininterface/proxy.h>
#include <plugininterface/addressparser.h>
#include <plugininterface/socketexceptions.h>
#include <exceptions/notimplemented.h>
#include "httpimp.h"
#include "settingsmanager.h"

HttpImp::HttpImp (QObject *parent)
: ImpBase (parent)
, Socket_ (0)
, Stop_ (false)
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

void HttpImp::run ()
{
	Socket_ = Proxy::Instance ()->MakeSocket ();
	Socket_->SetURL (URL_);
	Socket_->SetDefaultTimeout (SettingsManager::Instance ()->GetConnectTimeout ());

	try
	{
		Socket_->Connect (Socket_->GetAddressParser ()->GetHost (), Socket_->GetAddressParser ()->GetPort ());
	}
	catch (const Exceptions::Socket::BaseSocket& e)
	{
		emit error (e.GetReason ().c_str ());
		return;
	}

	WriteHeaders ();
	msleep (100);
	bool abort = ReadResponse ();

	if (abort)
	{
		delete Socket_;
		Socket_ = 0;
		emit stopped ();
		return;
	}

	Socket_->SetDefaultTimeout (SettingsManager::Instance ()->GetDefaultTimeout ());

	length_t counter = 0;

	int cacheSize = SettingsManager::Instance ()->GetCacheSize () * 1024; 
	SetCacheSize (cacheSize);
	Socket_->setReadBufferSize (cacheSize);

	AwaitFileInfoReaction_.second->lock ();
	AwaitFileInfoReaction_.first->wait (AwaitFileInfoReaction_.second);
	AwaitFileInfoReaction_.second->unlock ();

	emit dataFetched (RestartPosition_, Response_.ContentLength_ + RestartPosition_, QByteArray ());
	while (counter < Response_.ContentLength_)
	{
		QByteArray newData;
		try
		{
			newData = Socket_->ReadAll ();
		}
		catch (const Exceptions::Socket::SocketTimeout&)
		{
			emit error ("Main read loop: operation timed out :(");
			Stop_ = true;
		}
		catch (const Exceptions::Socket::BaseSocket& e)
		{
			qDebug () << Q_FUNC_INFO << "caught \"" << e.GetName ().c_str () << "\", saying\"" << e.GetReason ().c_str () << "\"";
			Stop_ = true;
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
	Socket_->Write ("GET " + Socket_->GetAddressParser ()->GetPath () + " HTTP/1.1\r\n");
	PairedStringList ua = SettingsManager::Instance ()->GetUserAgent ();
	QString agent = ua.first [ua.second];
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
	catch (const Exceptions::Socket::BaseSocket& e)
	{
		qDebug () << Q_FUNC_INFO << "caught \"" << e.GetName ().c_str () << "\", saying\"" << e.GetReason ().c_str () << "\"";
		return true;
	}
	ParseFirstLine (br);
	bool shouldWeReturn = DoPrimaryStuffWithResponse ();
	while (!shouldWeReturn)
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
		Response_.ContentLength_ = Response_.Fields_ ["content-length"].toULong ();
	shouldWeReturn = DoSecondaryStuffWithResponse ();

	QDateTime dt = QDateTime::fromString (Response_.Fields_ ["last-modified"].left (25), "ddd, dd MMM yyyy HH:mm:ss");
	RemoteFileInfo rfi = { true, dt, Response_.ContentLength_, Response_.Fields_ ["content-type"] };
	emit gotRemoteFileInfo (rfi);

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

bool HttpImp::DoPrimaryStuffWithResponse ()
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

		// Will be handled later
		case MovedPermanently:
		case TemporaryRedirect:
		case Found:
			return false;
		case RequestedRangeNotSatisfiable:
			emit finished ();
			return true;
		case BadRequest:
		case NotFound:
			return true;
		default:
			throw Exceptions::NotImplemented (QString ("The HTTP status code " + QString::number (Response_.StatusCode_) + " wasn't implemented yet. Please, send the bugreport to us with URL of file you've tried to download.").toStdString ());
	}
}

bool HttpImp::DoSecondaryStuffWithResponse ()
{
	switch (Response_.StatusCode_)
	{
		case MovedPermanently:
		case TemporaryRedirect:
		case Found:
			DoRedirect ();
			return true;
		default:
			return false;
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
}

void HttpImp::Finalize ()
{
	Socket_->Disconnect ();
	delete Socket_;
	Socket_ = 0;
}

