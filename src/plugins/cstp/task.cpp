#include "task.h"
#include <algorithm>
#include <typeinfo>
#include <stdexcept>
#include <QUrl>
#include <QHttp>
#include <QFtp>
#include <QFileInfo>
#include <QDataStream>
#include <QDir>
#include <QtDebug>
#include "hook.h"
#include "xmlsettingsmanager.h"

Task::Task (const QString& str)
: URL_ (str)
, Type_ (TInvalid)
, CurrentCmd_ (cmd_t (0, CInvalid))
, Done_ (0)
, Total_ (0)
, FileSizeAtStart_ (-1)
, Speed_ (0)
{
	StartTime_.start ();
	Construct ();
}

Task::~Task ()
{
}

struct HookTypeEqual
{
	Hook Hook_;

	HookTypeEqual (const Hook& hook)
	: Hook_ (hook)
	{
	}

	bool operator() (const Hook& hook) const
	{
		return typeid (Hook_) == typeid (hook);
	}
};

void Task::AddHook (const Hook& hook)
{
	if (std::find_if (Hooks_.begin (),
				Hooks_.end (),
				HookTypeEqual (hook)) == Hooks_.end ())
		Hooks_.push_back (hook);
}

void Task::RemoveHook (const Hook& hook)
{
	std::remove_if (Hooks_.begin (), Hooks_.end (), HookTypeEqual (hook));
}

void Task::Start (const boost::shared_ptr<QFile>& tof)
{
	FileSizeAtStart_ = tof->size ();
	Start (tof.get ());
}

void Task::Stop ()
{
	if (Type_ == THttp || Type_ == THttps)
		Http_->abort ();
	else if (Type_ == TFtp)
		Ftp_->abort ();
}

QByteArray Task::Serialize () const
{
	QByteArray result;
	{
		QDataStream out (&result, QIODevice::WriteOnly);
		out << 1
			<< URL_
			<< StartTime_
			<< Type_
			<< Done_
			<< Total_
			<< Speed_;
	}
	return result;
}

void Task::Deserialize (QByteArray& data)
{
	QDataStream in (&data, QIODevice::ReadOnly);
	int version = 0;
	in >> version;
	if (version == 1)
	{
		int type;
		in >> URL_
			>> StartTime_
			>> type
			>> Done_
			>> Total_
			>> Speed_;
		Type_ = static_cast<Type> (type);
		Construct ();
	}
	else
		throw std::runtime_error ("Unknown version");
}

double Task::GetSpeed () const
{
	return Speed_;
}

qint64 Task::GetDone () const
{
	return Done_;
}

qint64 Task::GetTotal () const
{
	return Total_;
}

QString Task::GetState () const
{
	if (Type_ == THttp || Type_ == THttps)
		return GetHTTPState ();
	else if (Type_ == TFtp)
		return GetFTPState ();
	else
		return tr ("Unknown task type");
}

QString Task::GetURL () const
{
	return URL_.toString ();
}

int Task::GetTimeFromStart () const
{
	return StartTime_.elapsed ();
}

bool Task::IsRunning () const
{
	if (Type_ == THttp || Type_ == THttps)
	{
		QHttp::State st = Http_->state ();
		return st != QHttp::Unconnected;
	}
	else if (Type_ == TFtp)
	{
		QFtp::State st = Ftp_->state ();
		return st != QFtp::Unconnected;
	}
	else
		return false;
}

QString Task::GetErrorString () const
{
	if (Type_ != TFtp)
		switch (Http_->error ())
		{
			case QHttp::NoError:
				return tr ("No error occurred.");
			case QHttp::HostNotFound:
				return tr ("The host name lookup failed.");
			case QHttp::ConnectionRefused:
				return tr ("The server refused the connection.");
			case QHttp::UnexpectedClose:
				return tr ("The server closed the connection unexpectedly.");
			case QHttp::InvalidResponseHeader:
				return tr ("The server sent an invalid response header.");
			case QHttp::WrongContentLength:
				return tr ("The client could not read the content correctly because an error with respect to the content length occurred.");
			case QHttp::Aborted:
				return tr ("The request was aborted with abort().");
			case QHttp::ProxyAuthenticationRequiredError:
				return tr ("QHttp is using a proxy, and the proxy server requires authentication to establish a connection.");
			case QHttp::AuthenticationRequiredError:
				return tr ("The web server requires authentication to complete the request.");
			case QHttp::UnknownError:
				return tr ("An error other than those specified above occurred.");
			default:
				return tr ("Unknown");
		}
	else
		return tr ("FTP shall be eliminated");
}

void Task::SetProxy (const QNetworkProxy& proxy)
{
	switch (Type_)
	{
		case THttp:
		case THttps:
			Http_->setProxy (proxy);
			break;
		case TFtp:
			// Doesn't matter, we'll abandon QFtp soon anyway.
			break;
		default:
			break;
	}
}

void Task::Start (QIODevice *to)
{
	StartTime_.restart ();
	if (Type_ == THttp || Type_ == THttps)
	{
		QHttp::ConnectionMode mode = QHttp::ConnectionModeHttp;
		if (Type_ == THttp)
			mode = QHttp::ConnectionModeHttp;
		else if (Type_ == THttps)
			mode = QHttp::ConnectionModeHttps;

		int id = Http_->setHost (URL_.host (),
				mode,
				(URL_.port () == -1) ? 0 : URL_.port ());
		Commands_.push_back (cmd_t (id, CConnect));

		QString ua = XmlSettingsManager::Instance ()
			.property ("UserUserAgent").toString ();
		if (ua.isEmpty ())
			ua = XmlSettingsManager::Instance ()
				.property ("PredefinedUserAgent").toString ();

		QString qSep;
		if (URL_.encodedQuery ().size ())
			qSep = "?";
		
		QHttpRequestHeader header ("GET", URL_.path () + qSep + URL_.encodedQuery ());
		header.setValue ("Host", URL_.host ());
		header.setValue ("Range", QString ("bytes=%1-").arg (to->size ()));
		header.setValue ("Accept", "*/*");
		header.setValue ("User-Agent", ua);
		header.setValue ("Referer", QString ("http://") + URL_.host ());
		id = Http_->request (header, 0, to);
		Commands_.push_back (cmd_t (id, CTransfer));
	}
	else if (Type_ == TFtp)
	{
		int id = Ftp_->connectToHost (URL_.host (),
				URL_.port () == -1 ? 21 : URL_.port ());
		Commands_.push_back (cmd_t (id, CConnect));

		QString login = URL_.userName ();
		QString password = URL_.password ();
		if (login.isEmpty ())
			login = XmlSettingsManager::Instance ()
				.property ("FTPLogin").toString ();
		if (password.isEmpty ())
			password = XmlSettingsManager::Instance ()
				.property ("FTPPassword").toString ();
		id = Ftp_->login (login, password);
		Commands_.push_back (cmd_t (id, CLogin));

		id = Ftp_->rawCommand ("TYPE I\r\n");
		Commands_.push_back (cmd_t (id, CTypeI));

		id = Ftp_->rawCommand ("REST " + QString::number (to->size ()));
		Commands_.push_back (cmd_t (id, CRest));

		QStringList exts = XmlSettingsManager::Instance ()
			.property ("TextTransferMode").toString ()
			.split (' ', QString::SkipEmptyParts);
		QFtp::TransferType ftt;
		if (exts.contains (QFileInfo (URL_.path ()).suffix ()))
			ftt = QFtp::Ascii;
		else
			ftt = QFtp::Binary;

		QFileInfo fi = QFileInfo (URL_.path ());

		id = Ftp_->cd (fi.dir ().path ());
		Commands_.push_back (cmd_t (id, CCD));

		id = Ftp_->get (fi.fileName (), to, ftt);
		Commands_.push_back (cmd_t (id, CTransfer));

		id = Ftp_->close ();
		Commands_.push_back (cmd_t (id, CDisconnect));
	}
}

void Task::Reset ()
{
	Commands_.clear ();
	Done_ = 0;
	Total_ = 0;
	Speed_ = 0;
	FileSizeAtStart_ = -1;
	Type_ = TInvalid;
	Http_.reset ();
	Ftp_.reset ();
}

void Task::Construct ()
{
	Reset ();
	if (!URL_.isValid ())
		return;

	QString scheme = URL_.scheme ();
	if (scheme == "ftp")
		ConstructFTP (scheme);
	else if (scheme == "http" || scheme == "https")
		ConstructHTTP (scheme);
}

void Task::ConstructFTP (const QString&)
{
	Ftp_.reset (new QFtp ());
	Type_ = TFtp;
	connect (Ftp_.get (),
			SIGNAL (done (bool)),
			this,
			SIGNAL (done (bool)));
	connect (Ftp_.get (),
			SIGNAL (done (bool)),
			this,
			SIGNAL (updateInterface ()));
	connect (Ftp_.get (),
			SIGNAL (stateChanged (int)),
			this,
			SIGNAL (updateInterface ()));
	connect (Ftp_.get (),
			SIGNAL (dataTransferProgress (qint64, qint64)),
			this,
			SLOT (handleDataTransferProgress (qint64, qint64)));
	connect (Ftp_.get (),
			SIGNAL (commandStarted (int)),
			this,
			SLOT (handleRequestStart (int)));
	connect (Ftp_.get (),
			SIGNAL (commandFinished (int, bool)),
			this,
			SLOT (handleRequestFinish (int, bool)));
}

void Task::ConstructHTTP (const QString& scheme)
{
	Http_.reset (new QHttp ());
	if (scheme == "http")
		Type_ = THttp;
	else if (scheme == "https")
		Type_ = THttps;
	connect (Http_.get (),
			SIGNAL (done (bool)),
			this,
			SIGNAL (done (bool)));
	connect (Http_.get (),
			SIGNAL (done (bool)),
			this,
			SIGNAL (updateInterface ()));
	connect (Http_.get (),
			SIGNAL (stateChanged (int)),
			this,
			SIGNAL (updateInterface ()));
	connect (Http_.get (),
			SIGNAL (dataReadProgress (int, int)),
			this,
			SLOT (handleDataTransferProgress (int, int)));
	connect (Http_.get (),
			SIGNAL (requestStarted (int)),
			this,
			SLOT (handleRequestStart (int)));
	connect (Http_.get (),
			SIGNAL (requestFinished (int, bool)),
			this,
			SLOT (handleRequestFinish (int, bool)));
	connect (Http_.get (),
			SIGNAL (responseHeaderReceived (const QHttpResponseHeader&)),
			this,
			SLOT (responseHeaderReceived (const QHttpResponseHeader&)));

	if (Type_ == THttps)
		connect (Http_.get (),
				SIGNAL (sslErrors (const QList<QSslError>&)),
				Http_.get (),
				SLOT (ignoreSslErrors ()));
}

struct CmdComparator
{
	int ID_;

	CmdComparator (int id)
	: ID_ (id)
	{
	}

	bool operator() (Task::cmd_t cmd) const
	{
		return ID_ == cmd.first;
	}
};

Task::cmd_t Task::FindCommand (int id) const
{
	cmds_t::const_iterator i = std::find_if (Commands_.begin (),
			Commands_.end (),
			CmdComparator (id));
	if (i != Commands_.end ())
		return *i;
	else
		return cmd_t (0, CUnknown);
}

void Task::RecalculateSpeed ()
{
	Speed_ = static_cast<double> (Done_ * 1000) / static_cast<double> (StartTime_.elapsed ());
}

QString Task::GetHTTPState () const
{
	if (!Http_.get ())
		return "HTTP is null";
	switch (Http_->state ())
	{
		case QHttp::Unconnected:
			return tr ("Unconnected");
		case QHttp::HostLookup:
			return tr ("Hostname lookup");
		case QHttp::Connecting:
			return tr ("Connecting");
		case QHttp::Sending:
			return tr ("Sending request");
		case QHttp::Reading:
			return tr ("Reading reply");
		case QHttp::Connected:
			return tr ("Connection is idle");
		case QHttp::Closing:
			return tr ("Closing");
		default:
			return tr ("Unknown");
	}
}

QString Task::GetFTPState () const
{
	if (!Ftp_.get ())
		return "FTP is null";
	switch (Ftp_->state ())
	{
		case QFtp::Unconnected:
			return tr ("Unconnected");
		case QFtp::HostLookup:
			return tr ("Hostname lookup");
		case QFtp::Connecting:
			return tr ("Connecting");
		case QFtp::Connected:
			return tr ("Connected");
		case QFtp::LoggedIn:
			return tr ("Logged in");
		case QFtp::Closing:
			return tr ("Closing");
		default:
			return tr ("Unknown");
	}
}

void Task::handleRequestStart (int id)
{
	CurrentCmd_ = FindCommand (id);
}

void Task::handleRequestFinish (int id, bool)
{
	std::remove_if (Commands_.begin (),
			Commands_.end (),
			CmdComparator (id));
}

void Task::handleDataTransferProgress (qint64 done, qint64 total)
{
	Done_ = done;
	Total_ = total;

	RecalculateSpeed ();

	emit updateInterface ();
}

void Task::handleDataTransferProgress (int done, int total)
{
	handleDataTransferProgress (static_cast<qint64> (done),
			static_cast<qint64> (total));
}

void Task::responseHeaderReceived (const QHttpResponseHeader& response)
{
	if (response.statusCode () == 301 ||
			response.statusCode () == 302)
	{
		Type_ = TInvalid;

		QString newUrl (response.value ("Location"));
		if (!QUrl (newUrl).isValid () ||
				RedirectHistory_.contains (newUrl, Qt::CaseInsensitive))
		{
			Http_->disconnect ();
			Http_->blockSignals (true);
			Http_->abort ();
			Http_.release ()->deleteLater ();
			qWarning () << Q_FUNC_INFO << "redirection failed, possibly a loop detected" << newUrl;
			return;
		}
		//Trying not to get into redirection loop
		else
			RedirectHistory_ << newUrl;
		
		QIODevice *to = Http_->currentDestinationDevice ();

		Http_->disconnect ();
		Http_->blockSignals (true);
		Http_->abort ();
		Http_.release ()->deleteLater ();

		QMetaObject::invokeMethod (this,
				"redirectedConstruction",
				Qt::QueuedConnection,
				Q_ARG (QIODevice*, to),
				Q_ARG (QString, newUrl));

	}
	else if (response.statusCode () == 504)
	{
		Http_->disconnect ();
		Http_->blockSignals (true);
		Http_->abort ();
		emit done (true);
	}
}

void Task::redirectedConstruction (QIODevice *to, const QString& newUrl)
{
	QFile *file = dynamic_cast<QFile*> (to);
	if (file && FileSizeAtStart_ >= 0)
	{
		file->close ();
		file->size ();
		file->resize (FileSizeAtStart_);
		file->open (QIODevice::ReadWrite);
	}

	URL_ = newUrl;
	Construct ();
	Start (file);
}

