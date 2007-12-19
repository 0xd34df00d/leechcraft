#include <QUrlInfo>
#include <QFileInfo>
#include <QDir>
#include <QMutex>
#include <QWaitCondition>
#include <plugininterface/tcpsocket.h>
#include <plugininterface/proxy.h>
#include <plugininterface/addressparser.h>
#include <plugininterface/socketexceptions.h>
#include <exceptions/notimplemented.h>
#include "ftpimp.h"
#include "settingsmanager.h"

namespace
{
	struct WrongLogin {};
	struct WrongPassword {};
	struct WrongPath {};
	struct WrongRestartPosition {};
	struct PasvFailed {};
	struct NlstFailed {};
	struct RetrFailed {};
	struct SizeFailed {};
	struct CwdFailed {};
	struct TypeIFailed {};
};

FtpImp::FtpImp (QObject *parent)
: ImpBase (parent)
, ControlSocket_ (0)
, DataSocket_ (0)
, Size_ (0)
, Result_ (0)
{
	Stop_ = false;
	GetFileSize_ = false;

	AwaitFileInfoReaction_.second = new QMutex;
	AwaitFileInfoReaction_.first = new QWaitCondition;
}

FtpImp::~FtpImp ()
{
	delete AwaitFileInfoReaction_.second;
	delete AwaitFileInfoReaction_.first;
}

void FtpImp::SetRestartPosition (ImpBase::length_t pos)
{
	RestartPosition_ = pos;
}

void FtpImp::SetURL (const QString& url)
{
	URL_ = url;
}

void FtpImp::run ()
{
	ControlSocket_ = Proxy::Instance ()->MakeSocket ();
	DataSocket_ = Proxy::Instance ()->MakeSocket ();
	DataSocket_->SetDefaultTimeout (SettingsManager::Instance ()->GetDefaultTimeout ());
	ControlSocket_->SetURL (URL_);
	ControlSocket_->SetDefaultTimeout (SettingsManager::Instance ()->GetConnectTimeout ());

	try
	{
		ControlSocket_->Connect (ControlSocket_->GetAddressParser ()->GetHost (), ControlSocket_->GetAddressParser ()->GetPort ());
	}
	catch (const Exceptions::Socket::BaseSocket& e)
	{
		qDebug () << Q_FUNC_INFO << "while trying to connect we caught" << e.GetReason ().c_str ();
		emit error ((QString (e.GetName ().c_str ())).append ("\r\n").append (e.GetReason ().c_str ()));
		Finalize ();
		emit stopped ();
		return;
	}

	ControlSocket_->SetDefaultTimeout (SettingsManager::Instance ()->GetDefaultTimeout ());

	try
	{
		if (!Negotiate ())
		{
			Finalize ();
			return;
		}
	}
	catch (WrongLogin)
	{
		emit error (tr ("Wrong login."));
		Finalize ();
		emit stopped ();
		return;
	}
	catch (WrongPassword)
	{
		emit error (tr ("Wrong password."));
		Finalize ();
		emit stopped ();
		return;
	}
	catch (WrongPath)
	{
		emit error (tr ("Wrong path, file not found."));
		Finalize ();
		emit stopped ();
		return;
	}
	catch (WrongRestartPosition)
	{
		emit error (tr ("Wrong restart position."));
		Finalize ();
		emit stopped ();
		return;
	}
	catch (PasvFailed)
	{
		emit error (tr ("PASV failed"));
		Finalize ();
		emit stopped ();
		return;
	}
	catch (NlstFailed)
	{
		emit error (tr ("NLST failed."));
		Finalize ();
		emit stopped ();
		return;
	}
	catch (RetrFailed)
	{
		emit error (tr ("RETR failed."));
		Finalize ();
		emit stopped ();
		return;
	}
	catch (SizeFailed)
	{
		emit error (tr ("SIZE failed."));
		Finalize ();
		emit stopped ();
		return;
	}
	catch (CwdFailed)
	{
		emit error (tr ("CWD failed."));
		Finalize ();
		emit stopped ();
		return;
	}
	catch (TypeIFailed)
	{
		emit error (tr ("TYPE I failed."));
		Finalize ();
		emit stopped ();
		return;
	}
	catch (const QStringList& files)
	{
		emit gotNewFiles (new QStringList (files));
		Finalize ();
		emit stopped ();
		return;
	}
	catch (const Exceptions::Generic& e)
	{
		qDebug () << Q_FUNC_INFO << "caught \"" << e.GetName ().c_str () << "\", saying\"" << e.GetReason ().c_str () << "\"";
		Finalize ();
		emit stopped ();
		return;
	}
	catch (...)
	{
		qDebug () << Q_FUNC_INFO << "caught something very strange.";
		Finalize ();
		emit stopped ();
		return;
	}

	int cacheSize = SettingsManager::Instance ()->GetCacheSize () * 1024;
	SetCacheSize (cacheSize);
	DataSocket_->setReadBufferSize (cacheSize);

	length_t counter = 0;

	QByteArray data;
	emit dataFetched (RestartPosition_, Size_ + RestartPosition_, data);

	while (Size_ > counter)
	{
		if (Stop_)
		{
			Stop_ = false;
			break;
		}

		QByteArray newData;
		try
		{
			newData = DataSocket_->ReadAll ();
		}
		catch (const Exceptions::Generic& e)
		{
			qDebug () << Q_FUNC_INFO << "caught \"" << e.GetName ().c_str () << "\", saying\"" << e.GetReason ().c_str () << "\"";
			break;
		}
		catch (...)
		{
			qDebug () << Q_FUNC_INFO << "caught some strange exception";
			break;
		}
		counter += newData.size ();
		Emit (counter + RestartPosition_, Size_, newData);
	}
	EmitFlush (counter + RestartPosition_, Size_);

	if (counter + RestartPosition_ == Size_)
		emit finished ();

	Finalize ();
}

void FtpImp::StopDownload ()
{
	Stop_ = true;
}

void FtpImp::ReactedToFileInfo ()
{
	AwaitFileInfoReaction_.first->wakeAll ();
}

void FtpImp::ScheduleGetFileSize ()
{
	if (!DataSocket_ || DataSocket_->state () == QAbstractSocket::UnconnectedState)
		GetFileSize_ = true;
}

bool FtpImp::Negotiate ()
{
	QString login = ControlSocket_->GetAddressParser ()->GetLogin ();
	if (login.isEmpty ())
		login = SettingsManager::Instance ()->GetResourceLogin ();
	QString password = ControlSocket_->GetAddressParser ()->GetPassword ();
	if (password.isEmpty ())
		password = SettingsManager::Instance ()->GetResourcePassword ();

	QFileInfo fileInfo (ControlSocket_->GetAddressParser ()->GetPath ());

	if (!HandleWelcome ())
		return false;
	if (!Login (login, password))
		return false;
	DoCwd (fileInfo.dir ().path ());
	DoPasv ();

	bool query = DoSize (fileInfo.fileName ());

	if (GetFileSize_)
	{
		GetFileSize_ = false;
		return false;
	}

	if (query)
		DoQuery (fileInfo);
	else
		DoGetFileInfo (fileInfo.fileName ());

	DoInitTransfer (fileInfo.fileName ());

	return true;
}

bool FtpImp::HandleWelcome ()
{
	ReadCtrlResponse ();
	if (Result_ == 421)
	{
		emit enqueue ();
		return false;
	}
	return true;
}

void FtpImp::DoCwd (const QString& dir)
{
	ControlSocket_->Write (QString ("SYST\r\n"), false);
	ReadCtrlResponse ();

	ControlSocket_->Write (QString ("PWD\r\n"), false);
	ReadCtrlResponse ();

	ControlSocket_->Write (QString ("TYPE I\r\n"), false);
	ReadCtrlResponse ();

	ControlSocket_->Write ("CWD " + dir + "\r\n", false);
	if (ReadCtrlResponse () != 250)
		throw CwdFailed ();
}

bool FtpImp::DoSize (const QString& dir)
{
	ControlSocket_->Write ("SIZE " + dir + "\r\n");
	if (ReadCtrlResponse () != 213)
		return true;
	else
	{
		Size_ = LastReply_.remove (0, 4).trimmed ().toLongLong ();
		emit gotFileSize (Size_);
		return false;
	}
}

void FtpImp::DoGetFileInfo (const QString& file)
{
	ControlSocket_->Write ("MDTM " + file + "\r\n");
	if (ReadCtrlResponse () == 213)
		Modification_ = QDateTime::fromString (LastReply_.remove (0, 4).trimmed (), "yyyyMMddHHmmss");
	RemoteFileInfo rfi = { true, Modification_, Size_, "" };
	emit gotRemoteFileInfo (rfi);

	AwaitFileInfoReaction_.second->lock ();
	AwaitFileInfoReaction_.first->wait (AwaitFileInfoReaction_.second);
	AwaitFileInfoReaction_.second->unlock ();
}

void FtpImp::DoQuery (const QFileInfo& fileInfo)
{
	ControlSocket_->Write ("NLST " + fileInfo.fileName () + "\r\n", false);
	ReadCtrlResponse ();
	if (Result_ == 501)
	{
		return;
	}
	else if (Result_ == 150 || Result_ == 125)
	{
		QString listing = ""; 
		try
		{
			listing = DataSocket_->ReadAll ().trimmed ();
		}
		catch (...) {}
		if (listing.isEmpty ())
			throw WrongPath ();
		QStringList files = listing.split ("\r\n");
		if (files.size () >= 1)
		{
			for (int i = 0; i < files.size (); ++i)
				files [i].prepend ("/").prepend ((fileInfo.dir ().path ()));
			throw files;
		}
		try
		{
			if (ReadCtrlResponse () != 226)
				qDebug () << "Some shit with NLST, but ignoring it.";
		}
		catch (const Exceptions::Socket::BaseSocket& e)
		{
			qDebug () << Q_FUNC_INFO << "caught \"" << e.GetName ().c_str () << "\", saying\"" << e.GetReason ().c_str () << "\"";
			qDebug () << Q_FUNC_INFO << "Seems like server says nothing about successful ops, that's bad";
		}
	}
	else
		throw NlstFailed ();
}

void FtpImp::DoInitTransfer (const QString& filename)
{
	ControlSocket_->Write (QString ("TYPE I\r\n"), false);
	if (ReadCtrlResponse () != 200)
		throw TypeIFailed ();
	ControlSocket_->Write ("REST " + QString::number (RestartPosition_) + "\r\n", false);
	if (ReadCtrlResponse () != 350)
		throw WrongRestartPosition ();
	ControlSocket_->Write (QString ("TYPE I\r\n"), false);
	if (ReadCtrlResponse () != 200)
		throw TypeIFailed ();

	DoPasv ();

	ControlSocket_->Write ("RETR " + filename + "\r\n", false);
	ReadCtrlResponse ();
	if (Result_ != 150 && Result_ != 125)
		throw RetrFailed ();
	else if (!Size_)
	{
		int sizeOffsetStart = LastReply_.indexOf ('(');
		int sizeOffsetEnd = LastReply_.indexOf (' ', sizeOffsetStart);
		QString bytes = LastReply_.mid (sizeOffsetStart + 1, sizeOffsetEnd - sizeOffsetStart - 1);
		Size_ = bytes.toLongLong () + RestartPosition_;
	}
}

bool FtpImp::Login (const QString& login, const QString& password)
{
	QList<int> goodReplyList;
	goodReplyList << 230 << 220;

	ControlSocket_->Write ("USER " + login + "\r\n", false);
	ReadCtrlResponse ();
	if (Result_ == 331 || Result_ == 220)
	{
		ControlSocket_->Write ("PASS " + password + "\r\n", false);
		ReadCtrlResponse ();
		if (Result_ == 421)
		{
			emit enqueue ();
			return false;
		}
		else if (Result_ != 230 && Result_ != 220)
			throw WrongPassword ();

	}
	else if (Result_ == 230)
		return true;
	else if (Result_ == 421)
	{
		emit enqueue ();
		return false;
	}
	else
		throw WrongLogin ();

	return true;
}

int FtpImp::ReadCtrlResponse ()
{
	LastReply_ = ControlSocket_->ReadLine ();
	Result_ = LastReply_.left (3).toInt ();
	bool continued = (LastReply_ [3] == '-');
	while (continued)
	{
		LastReply_ = ControlSocket_->ReadLine ();
		continued = (LastReply_ [3] == '-');
	}
	return Result_;
}

void FtpImp::DoPasv ()
{
	ControlSocket_->Write (QString ("PASV\r\n"));
	if (ReadCtrlResponse () != 227)
		throw Exceptions::NotImplemented (MESSAGE (" modes other than passive are not implemented yet."));
	else
		PasvHandler ();
}

void FtpImp::PasvHandler ()
{
	QString pasv = LastReply_;
	pasv = pasv.section ('(', 1).section (')', 0, 0);
	QStringList bytes = pasv.split (',');
	int port = (bytes [4].toInt () << 8) + bytes [5].toInt ();
	try
	{
		if (DataSocket_->state () != QAbstractSocket::UnconnectedState)
			DataSocket_->Disconnect ();
	}
	catch (...)
	{
	}
	DataSocket_->Connect (QString (bytes [0] + "." + bytes [1] + "." + bytes [2] + "." + bytes [3]), port);
}

void FtpImp::Finalize ()
{
	DataSocket_->SetDefaultTimeout (SettingsManager::Instance ()->GetStopTimeout ());
	try
	{
		if (DataSocket_->state () != QAbstractSocket::UnconnectedState)
			DataSocket_->Disconnect ();
	}
	catch (const Exceptions::Generic& e)
	{
		qDebug () << Q_FUNC_INFO << "caught \"" << e.GetName ().c_str () << "\", saying\"" << e.GetReason ().c_str () << "\"";
	}
	catch (...)
	{ }

	try
	{
		ControlSocket_->Write (QString ("QUIT\r\n"), false);
		ReadCtrlResponse ();
	}
	catch (const Exceptions::Generic& e)
	{
		qDebug () << Q_FUNC_INFO << "caught \"" << e.GetName ().c_str () << "\", saying\"" << e.GetReason ().c_str () << "\"";
	}
	catch (...)
	{ }

	DataSocket_->SetDefaultTimeout (SettingsManager::Instance ()->GetStopTimeout ());
	try
	{
		if (ControlSocket_->state () != QAbstractSocket::UnconnectedState)
			ControlSocket_->Disconnect ();
	}
	catch (const Exceptions::Generic& e)
	{
		qDebug () << Q_FUNC_INFO << "caught \"" << e.GetName ().c_str () << "\", saying\"" << e.GetReason ().c_str () << "\"";
	}
	catch (...)
	{ }

	delete ControlSocket_;
	delete DataSocket_;
	ControlSocket_ = 0;
	DataSocket_ = 0;
}

