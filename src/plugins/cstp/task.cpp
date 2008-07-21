#include "task.h"
#include <algorithm>
#include <typeinfo>
#include <QUrl>
#include <QHttp>
#include <QFtp>
#include "hook.h"

Task::Task (const QString& str)
: Http_ (0)
, Ftp_ (0)
, URL_ (str)
, Type_ (TInvalid)
, CurrentCmd_ (cmd_t (0, CInvalid))
{
	Construct ();
}

Task::~Task ()
{
	delete Http_;
	delete Ftp_;
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

void Task::Start (QIODevice *to)
{
	if (Type_ == THttp || Type_ == THttps)
	{
		QHttp::ConnectionMode mode;
		if (Type_ == THttp)
			mode = QHttp::ConnectionModeHttp;
		else if (Type_ == THttps)
			mode = QHttp::ConnectionModeHttps;

		int id = Http_->setHost (URL_.host (),
				mode,
				(URL_.port () == -1) ? 0 : URL_.port ());
		Commands_.push_back (cmd_t (id, CConnect));
		
		QHttpRequestHeader header ("GET", URL_.path ());
		header.setValue ("Host", URL_.host ());
		header.setValue ("Range", QString ("bytes=%1-").arg (to->size ()));
		header.setValue ("Accept", "*/*");
		header.setValue ("User-Agent", "ShittyCrap");		// FIXME
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
			login = "anonymous";							// FIXME
		if (password.isEmpty ())
			password = "default@password.com";				// FIXME
		id = Ftp_->login (login, password);
		Commands_.push_back (cmd_t (id, CLogin));

		id = Ftp_->rawCommand ("TYPE I\r\n");
		Commands_.push_back (cmd_t (id, CTypeI));

		id = Ftp_->rawCommand ("REST " + QString::number (to->size ()));
		Commands_.push_back (cmd_t (id, CRest));

		id = Ftp_->get (URL_.path (), to);					// FIXME transfer type
		Commands_.push_back (cmd_t (id, CTransfer));

		id = Ftp_->close ();
		Commands_.push_back (cmd_t (id, CDisconnect));
	}
}

void Task::Stop ()
{
	if (Type_ == THttp || Type_ == THttps)
		Http_->abort ();
	else if (Type_ == TFtp)
		Ftp_->abort ();

	URL_ = QUrl ();
	Construct ();
}

void Task::Construct ()
{
	Type_ = TInvalid;
	delete Http_;
	delete Ftp_;
	Http_ = 0;
	Ftp_ = 0;
	if (!URL_.isValid ())
		return;

	QString scheme = URL_.scheme ();
	if (scheme == "ftp")
	{
		Ftp_ = new QFtp (this);
		Type_ = TFtp;
		connect (Ftp_,
				SIGNAL (done (bool)),
				this,
				SIGNAL (done (bool)));
		connect (Ftp_,
				SIGNAL (stateChanged (int)),
				this,
				SIGNAL (stateChanged (int)));
		connect (Ftp_,
				SIGNAL (dataTransferProgress (qint64, qint64)),
				this,
				SIGNAL (dataTransferProgress (qint64, qint64)));
		connect (Ftp_,
				SIGNAL (commandStarted (int)),
				this,
				SLOT (handleRequestStart (int)));
		connect (Ftp_,
				SIGNAL (commandFinished (int, bool)),
				this,
				SLOT (handleRequestFinish (int, bool)));
	}
	else if (scheme == "http" || scheme == "https")
	{
		Http_ = new QHttp (this);
		if (scheme == "http")
			Type_ = THttp;
		else if (scheme == "https")
			Type_ = THttps;
		connect (Http_,
				SIGNAL (done (bool)),
				this,
				SLOT (done (bool)));
		connect (Http_,
				SIGNAL (stateChanged (int)),
				this,
				SIGNAL (stateChanged (int)));
		connect (Http_,
				SIGNAL (dataReadProgress (int, int)),
				this,
				SIGNAL (dataTransferProgress (qint64, qint64)));
		connect (Http_,
				SIGNAL (requestStarted (int)),
				this,
				SLOT (handleRequestStart (int)));
		connect (Http_,
				SIGNAL (requestFinished (int, bool)),
				this,
				SLOT (handleRequestFinish (int, bool)));
	}
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

void Task::handleRequestStart (int id)
{
	CurrentCmd_ = FindCommand (id);
}

void Task::handleRequestFinish (int id, bool result)
{
	std::remove_if (Commands_.begin (),
			Commands_.end (),
			CmdComparator (id));
}

