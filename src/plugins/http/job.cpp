#include <QtCore/QtCore>
#include <QtDebug>
#include <QMessageBox>
#include <plugininterface/addressparser.h>
#include <plugininterface/tcpsocket.h>
#include <plugininterface/proxy.h>
#include <exceptions/logic.h>
#include <exceptions/situative.h>
#include <exceptions/io.h>
#include "job.h"
#include "jobparams.h"
#include "jobrepresentation.h"
#include "fileexistsdialog.h"
#include "xmlsettingsmanager.h"
#include "jobmanager.h"
#include "httpimp.h"
#include "ftpimp.h"

Job::Job (JobParams *params, QObject *parent)
: QObject (parent)
, ErrorFlag_ (false)
, GetFileSize_ (false)
, Speed_ (0)
, CurrentSpeed_ (0)
, DownloadTime_ (params->DownloadTime_)
, DownloadedSize_ (0)
, TotalSize_ (params->Size_)
, RestartPosition_ (0)
, PreviousDownloadSize_ (params->Size_)
, ProtoImp_ (0)
, Params_ (params)
, File_ (0)
, State_ (StateIdle)
, JobType_ (File)
{
	qRegisterMetaType<ImpBase::RemoteFileInfo> ("ImpBase::RemoteFileInfo");
	StartTime_ = new QTime;
	UpdateTime_ = new QTime;
	FillErrorDictionary ();
	FileExistsDialog_ = new FileExistsDialog (parent ? qobject_cast<JobManager*> (parent)->GetTheMain () : 0);

	if (Params_->IsFullName_)
	{
		if (!QFileInfo (Params_->LocalName_).dir ().exists ())
			Params_->LocalName_ = QDir::homePath () + "/" + QFileInfo (Params_->LocalName_).fileName ();
	}
	else
		if (!QFileInfo (Params_->LocalName_).exists ())
			Params_->LocalName_ = QDir::homePath () + "/";
}

Job::~Job ()
{
	delete StartTime_;
	delete UpdateTime_;
	delete ProtoImp_;
	delete File_;
	delete Params_;
}

void Job::DoDelayedInit ()
{
	if (Params_->LocalName_.endsWith ('/') || Params_->LocalName_.endsWith ('\\'))
		Params_->LocalName_.remove (Params_->LocalName_.size () - 1, 1);

	QString filename = MakeFilename ();
	if (QFile::exists (filename))
	{
		QFile *file = new QFile (filename);
		HttpImp::length_t size = file->size ();
		delete file;
		processData (size, TotalSize_, QByteArray ());
	}
	emit updateDisplays (ID_);
}

void Job::SetID (unsigned int id)
{
	ID_ = id;
}

unsigned int Job::GetID () const
{
	return ID_;
}

JobRepresentation* Job::GetRepresentation () const
{
	JobRepresentation *jr		= new JobRepresentation;
	jr->ID_						= ID_;
	jr->URL_					= Params_->URL_;
	jr->LocalName_				= MakeFilename ();
	jr->Speed_					= Speed_;
	jr->CurrentSpeed_			= CurrentSpeed_;
	jr->Downloaded_				= DownloadedSize_;
	jr->AverageTime_			= (TotalSize_ - DownloadedSize_) / Speed_;
	jr->CurrentTime_			= (TotalSize_ - DownloadedSize_) / CurrentSpeed_;
	jr->Size_					= TotalSize_;
	jr->ShouldBeSavedInHistory_ = Params_->ShouldBeSavedInHistory_;
	jr->DownloadTime_			= DownloadTime_ + (GetState () == StateDownloading ? (StartTime_->elapsed () > 0 ? StartTime_->elapsed () : 0) : 0);
	return jr;
}

bool Job::GetErrorFlag ()
{
	bool result = ErrorFlag_;
	return result;
}

QString Job::GetErrorReason ()
{
	QString result = ErrorReason_;
	return result;
}

void Job::Start ()
{
	State_ = StateDownloading;
	QString ln = MakeFilename ();
	QFileInfo fileInfo (ln);
	if (fileInfo.exists () && !fileInfo.isDir ())
		RestartPosition_ = QFile (ln).size ();

	delete File_;
	File_ = 0;
	delete ProtoImp_;

	if (Params_->URL_.left (3).toLower () == "ftp")
		ProtoImp_ = new FtpImp ();
	else
		ProtoImp_ = new HttpImp ();

	connect (ProtoImp_, SIGNAL (gotNewFiles (QStringList*)), this, SLOT (handleNewFiles (QStringList*)));
	connect (ProtoImp_, SIGNAL (clarifyURL (QString)), this, SLOT (handleClarifyURL (QString)));
	connect (ProtoImp_, SIGNAL (dataFetched (ImpBase::length_t, ImpBase::length_t, QByteArray)), this, SLOT (processData (ImpBase::length_t, ImpBase::length_t, QByteArray)), Qt::DirectConnection);
	connect (ProtoImp_, SIGNAL (finished ()), this, SLOT (reemitFinished ()), Qt::DirectConnection);
	connect (ProtoImp_, SIGNAL (error (QString)), this, SLOT (handleShowError (QString)));
	connect (ProtoImp_, SIGNAL (stopped ()), this, SLOT (reemitStopped ()));
	connect (ProtoImp_, SIGNAL (enqueue ()), this, SLOT (reemitEnqueue ()));
	connect (ProtoImp_, SIGNAL (gotRemoteFileInfo (const ImpBase::RemoteFileInfo&)), this, SLOT (handleRemoteFileInfo (const ImpBase::RemoteFileInfo&)), Qt::QueuedConnection);
	connect (ProtoImp_, SIGNAL (gotFileSize (ImpBase::length_t)), this, SLOT (reemitGotFileSize (ImpBase::length_t)));

	ProtoImp_->SetURL (Params_->URL_);
	ProtoImp_->SetRestartPosition (RestartPosition_);
	if (GetFileSize_)
		ProtoImp_->ScheduleGetFileSize ();
	GetFileSize_ = false;

	DownloadedSize_ = RestartPosition_;
	TotalSize_ = RestartPosition_;

	ProtoImp_->StartDownload ();
	StartTime_->start ();
}

void Job::GetFileSize ()
{
	GetFileSize_ = true;
	Start ();
}

void Job::handleRemoteFileInfo (const ImpBase::RemoteFileInfo& rfi)
{
	QString ln = MakeFilename ();
	QFileInfo fileInfo (ln);
	if (fileInfo.exists () && !fileInfo.isDir ())
	{
		if (rfi.Modification_ >= fileInfo.lastModified ())
		{
			if (QMessageBox::question (parent () ? qobject_cast<JobManager*> (parent ())->GetTheMain () : 0, tr ("Question."), tr ("File on remote server is newer than local. Should I redownload it from scratch or just leave it alone?"), QMessageBox::Ok | QMessageBox::Cancel) == QMessageBox::Ok)
			{
				File_ = new QFile (ln);
				if (!File_->remove ())
				{
					if (!File_->open (QFile::Truncate))
						throw Exceptions::IO (tr ("File could be neither removed, nor truncated. Check your rights or smth.").toStdString ());
					File_->close ();
				}
				File_->open (QFile::WriteOnly);
				RestartPosition_ = 0;
				Stop ();
				Start ();
				return;
			}
			else
			{
				Stop ();
				return;
			}
		}
		else
		{
			FileExistsDialog_->exec ();
			QFile f (ln);
			switch (FileExistsDialog_->GetSelected ())
			{
				case FileExistsDialog::Scratch:
					if (!QFile::remove (ln))
					{
						if (!f.open (QFile::Truncate))
							throw Exceptions::IO (tr ("File could be neither removed, nor truncated. Check your rights or smth.").toStdString ());
						f.close ();
					}
					break;
				case FileExistsDialog::Continue:
					RestartPosition_ = f.size ();
					break;
				case FileExistsDialog::Unique:
					Params_->LocalName_ = MakeUniqueNameFor (ln);
					break;
				case FileExistsDialog::Abort:
					ProtoImp_->ReactedToFileInfo ();
					Stop ();
					return;
			}
		}
	}
	else if (fileInfo.isDir ())
	{
	}
	File_ = new QFile (MakeFilename ());
	ProtoImp_->ReactedToFileInfo ();
}

void Job::Stop ()
{
	if (State_ != StateIdle && ProtoImp_ && ProtoImp_->isRunning ())
	{
		ProtoImp_->StopDownload ();
		if (!ProtoImp_->wait (XmlSettingsManager::Instance ()->property ("DisconnectTimeout").toInt ()))
			ProtoImp_->terminate ();
		reemitStopped ();
	}
	State_ = StateIdle;
}

void Job::Release ()
{
	if (State_ != StateIdle && ProtoImp_ && ProtoImp_->isRunning ())
	{
		ProtoImp_->StopDownload ();
		if (!ProtoImp_->wait (XmlSettingsManager::Instance ()->property ("DisconnectTimeout").toInt ()))
			ProtoImp_->terminate ();
	}
	delete ProtoImp_;
	ProtoImp_ = 0;
}

Job::State Job::GetState () const
{
	return State_;
}

void Job::UpdateParams (JobParams *p)
{
	Params_->URL_ = p->URL_;
	Params_->LocalName_ = p->LocalName_;
	Params_->IsFullName_ = true;
	emit updateDisplays (GetID ());
}

void Job::handleNewFiles (QStringList *files)
{
	QFileInfo fileInfo (Params_->LocalName_);
	QDir dir2create (QDir::root ());
	if (!fileInfo.exists () && !dir2create.mkpath (Params_->LocalName_))
	{
		emit showError (Params_->URL_, QString (tr ("Could not create directory<br /><code>%1</code><br /><br />Stopping work.")).arg (Params_->LocalName_));
		ProtoImp_->StopDownload ();
	}

	AddressParser *ap = TcpSocket::GetAddressParser (Params_->URL_);
	QString constructedURL = "ftp://" + ap->GetHost () + ":" + QString::number (ap->GetPort ());
	if (Params_->LocalName_.right (1) != "/")
		Params_->LocalName_ += "/";
	delete ap;
	for (int i = 0; i < files->size (); ++i)
	{
		JobParams *jp = new JobParams;
		if (files->at (i) [0] != '/')
			jp->URL_ = constructedURL + "/" + files->at (i);
		else
			jp->URL_ = constructedURL + files->at (i);
		jp->LocalName_ = Params_->LocalName_ + QFileInfo (files->at (i)).fileName ();
		jp->IsFullName_ = true;
		jp->Autostart_ = XmlSettingsManager::Instance ()->property ("AutostartChildren").toBool ();
		jp->ShouldBeSavedInHistory_ = true;
		jp->Size_ = 0;
		jp->DownloadTime_ = 0;
		emit addJob (jp);
	}

	emit deleteJob (GetID ());
}

void Job::handleClarifyURL (QString url)
{
	if (!ProtoImp_->wait (XmlSettingsManager::Instance ()->property ("DisconnectTimeout").toInt ()))
		ProtoImp_->terminate ();
	delete ProtoImp_;
	ProtoImp_ = 0;
	Params_->URL_ = url;
	Start ();
	emit started (GetID ());
}

void Job::processData (ImpBase::length_t ready, ImpBase::length_t total, QByteArray newData)
{
	if (newData.isEmpty () || newData.isNull ())
	{
		PreviousDownloadSize_ = ready;
		DownloadedSize_ = ready;
		TotalSize_ = total;
		StartTime_->restart ();
		UpdateTime_->restart ();
		emit updateDisplays (GetID ());
		return;
	}
	else if (!File_)
		return;

	DownloadedSize_ = ready;
	TotalSize_ = total;
	Speed_ = (DownloadedSize_ - RestartPosition_) / static_cast<double> (StartTime_->elapsed ()) * 1000;

	if (UpdateTime_->elapsed () > XmlSettingsManager::Instance ()->property ("InterfaceUpdateTimeout").toInt ())
	{
		CurrentSpeed_ = (DownloadedSize_ - PreviousDownloadSize_) / static_cast<double> (UpdateTime_->elapsed ()) * 1000;
		PreviousDownloadSize_ = DownloadedSize_;
		UpdateTime_->restart ();
		emit updateDisplays (GetID ());
	}

	if (!File_->open (QFile::WriteOnly | QFile::Append))
	{
		QTemporaryFile tmpFile ("leechcraft.httpplugin.XXXXXX");
		emit showError (Params_->URL_, QString (tr ("Could not open file for write/append<br /><code>%1</code>"
						"<br /><br />Flushing cache to temp file<br /><code>%2</code><br />and stopping work."))
						.arg (File_->fileName ())
						.arg (tmpFile.fileName ()));

		if (!tmpFile.open ())
		{
			emit showError (Params_->URL_, QString (tr ("Could not open temporary file for write<br /><code>%1</code>"))
						.arg (tmpFile.fileName ()));
		}
		else
		{
			tmpFile.write (newData);
			tmpFile.close ();
			tmpFile.setAutoRemove (false);
		}

		ProtoImp_->StopDownload ();
	}
	else
	{
		File_->write (newData);
		File_->close ();
	}
}

void Job::reemitFinished ()
{
	emit finished (GetID ());
	State_ = StateIdle;
	DownloadTime_ += StartTime_->elapsed ();
	StartTime_->restart ();
}

void Job::handleShowError (QString error)
{
	emit showError (Params_->URL_, error);
}

void Job::reemitStopped ()
{
	qDebug () << Q_FUNC_INFO;
	State_ = StateIdle;
	emit stopped (GetID ());
	State_ = StateIdle;
	DownloadTime_ += StartTime_->elapsed ();
	StartTime_->restart ();
}

void Job::reemitEnqueue ()
{
	disconnect (this, SIGNAL (stopped (unsigned int)), 0, 0);
	Stop ();
	emit enqueue (GetID ());
	State_ = StateWaiting;
	DownloadTime_ += StartTime_->elapsed ();
	StartTime_->restart ();
}

void Job::reemitGotFileSize (ImpBase::length_t size)
{
	TotalSize_ = size;
	emit gotFileSize (GetID ());
}

void Job::FillErrorDictionary ()
{
	ErrorDictionary_ [QAbstractSocket::ConnectionRefusedError] = tr ("Connection refused");
	ErrorDictionary_ [QAbstractSocket::RemoteHostClosedError] = tr ("Remote host closed connection");
	ErrorDictionary_ [QAbstractSocket::HostNotFoundError] = tr ("Host not found");
	ErrorDictionary_ [QAbstractSocket::SocketAccessError] = tr ("Socket access error");
	ErrorDictionary_ [QAbstractSocket::SocketResourceError] = tr ("Socker resource error");
	ErrorDictionary_ [QAbstractSocket::SocketTimeoutError] = tr ("Socket timed out");
	ErrorDictionary_ [QAbstractSocket::DatagramTooLargeError] = tr ("Datagram too large");
	ErrorDictionary_ [QAbstractSocket::NetworkError] = tr ("Network error");
	ErrorDictionary_ [QAbstractSocket::AddressInUseError] = tr ("Address already in use");
	ErrorDictionary_ [QAbstractSocket::SocketAddressNotAvailableError] = tr ("Socket address not available");
	ErrorDictionary_ [QAbstractSocket::UnsupportedSocketOperationError] = tr ("Unsupported socket operation");
	ErrorDictionary_ [QAbstractSocket::UnfinishedSocketOperationError] = tr ("Unfinished socket operation");
	ErrorDictionary_ [QAbstractSocket::ProxyAuthenticationRequiredError] = tr ("Proxy autentication required");
	ErrorDictionary_ [QAbstractSocket::UnknownSocketError] = tr ("Unknown socket error");
}

QString Job::MakeUniqueNameFor (const QString& name)
{
	QString result = name;
	int i = 0;
	while (QFile (result + QString::number (i++)).exists ());
	return result + QString::number (i - 1);
}

QString Job::MakeFilename () const
{
	static AddressParser *ap = TcpSocket::GetAddressParser ("");
	if (!Params_->IsFullName_)
	{
		ap->Reparse (Params_->URL_);
		if (!(Params_->LocalName_.at (Params_->LocalName_.size () - 1) == '/'))
			Params_->LocalName_.append ('/');
		Params_->LocalName_.append (QFileInfo (ap->GetPath ()).fileName ());
		Params_->IsFullName_ = true;
	}

	Params_->LocalName_ = QUrl::fromPercentEncoding (Params_->LocalName_.toUtf8 ());

	return Params_->LocalName_;
}

