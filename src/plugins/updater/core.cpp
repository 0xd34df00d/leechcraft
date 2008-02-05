#include <QtCore/QtCore>
#include <QtXml/QtXml>
#include <QtDebug>
#include <interfaces/interfaces.h>
#include <plugininterface/proxy.h>
#include "xmlsettingsmanager.h"
#include "core.h"

Core::Core ()
: Waiter_ (qMakePair (new QMutex, new QWaitCondition))
, CheckWaiter_ (qMakePair (new QMutex, new QWaitCondition))
, DownloadWaiter_ (qMakePair (new QMutex, new QWaitCondition))
, ShouldQuit_ (false)
, GotUpdateInfoFile_ (false)
, CheckState_ (NotChecking)
, DownloadState_ (NotDownloading)
{
}

Core::~Core ()
{
	delete Waiter_.first;
	delete Waiter_.second;
	delete CheckWaiter_.first;
	delete CheckWaiter_.second;
	delete DownloadWaiter_.first;
	delete DownloadWaiter_.second;
}

void Core::Release ()
{
	ShouldQuit_ = true;
	Waiter_.second->wakeOne ();
}

void Core::SetProvider (QObject* provider, const QString& feature)
{
	Providers_ [feature.toLower ()] = provider;
	if (feature.toLower () == "ftp" || feature.toLower () == "http")
	{
		connect (provider, SIGNAL (jobFinished (int)), this, SLOT (handleDownloadFinished (int)));
		connect (provider, SIGNAL (jobRemoved (int)), this, SLOT (handleDownloadRemoved (int)));
		connect (provider, SIGNAL (jobError (int, IDirectDownload::Error)), this, SLOT (handleDownloadError (int, IDirectDownload::Error)));
		connect (provider, SIGNAL (jobProgressUpdated (int, int)), this, SLOT (handleDownloadProgressUpdated (int, int)));
	}
}

bool Core::IsChecking () const
{
	return (CheckState_ == ShouldCheck || CheckState_ == Checking);
}

bool Core::IsDownloading () const
{
	return (DownloadState_ == ShouldDownload || DownloadState_ == Downloading);
}

void Core::checkForUpdates ()
{
	if (IsChecking () || IsDownloading ())
		return;

	CheckState_ = ShouldCheck;
	Waiter_.second->wakeOne ();
}

void Core::downloadUpdates (const QList<int>& ids)
{
	if (IsChecking () || IsDownloading ())
		return;

	DownloadState_ = ShouldDownload;
	IDs2Download_ = ids;
	Waiter_.second->wakeOne ();
}

void Core::handleDownloadFinished (int id)
{
	if (id == UpdateInfoID_)
	{
		CheckState_ = CheckedSuccessfully;
		CheckWaiter_.second->wakeAll ();
	}
	else if (id == DownloadFileID_)
		DownloadWaiter_.second->wakeAll ();
}

void Core::handleDownloadRemoved (int)
{
}

void Core::handleDownloadError (int, IDirectDownload::Error)
{
}

void Core::handleDownloadProgressUpdated (int, int)
{
}

void Core::versionDownloadAdded (int id)
{
	UpdateInfoID_ = id;
}

void Core::fileDownloadAdded (int id)
{
	qDebug () << Q_FUNC_INFO;
	DownloadFileID_ = id;
}

void Core::run ()
{
	forever
	{
		Waiter_.first->lock ();
		Waiter_.second->wait (Waiter_.first);

		if (CheckState_ == ShouldCheck)
			Check ();
		if (DownloadState_ == ShouldDownload)
			Download ();
		if (ShouldQuit_)
			break;

		emit finishedLoop ();

		Waiter_.first->unlock ();
	}
}

bool Core::Check ()
{
	CheckState_ = Checking;
	QString mirror = XmlSettingsManager::Instance ()->property ("Mirror").toString ();
	bool result = false;
	QObject *provider;
	if (mirror.left (6).toLower () == "ftp://")
		provider = Providers_ ["ftp"];
	else if (mirror.left (7).toLower () == "http://")
		provider = Providers_ ["http"];
	else
	{
		emit error ("Wrong mirror");
		return false;
	}

	if (!provider)
	{
		emit error ("Could not find satisfying provider");
		return false;
	}

	if (!mirror.endsWith ('/'))
		mirror.append ('/');

	result = HandleSingleMirror (provider, mirror);


	if (!result)
	{
		emit error ("Queried all mirrors, but still no luck.");
		CheckState_ = CheckError;
		return false;
	}

	CheckState_ = CheckedSuccessfully;

	if (Parse ())
	{
		emit finishedCheck ();
		return true;
	}
	else
		return false;
}

void Core::Download ()
{
	/*
	ToApply_.clear ();
	QString mirror = XmlSettingsManager::Instance ()->property ("Mirror").toString ();
	for (int i = 0; i < IDs2Download_.size (); ++i)
	{
//		FileRep rep = Files_.at (IDs2Download_.at (i));
		QObject *provider;
		if (mirror.left (6).toLower () == "ftp://")
			provider = Providers_ ["ftp"];
		else if (mirror.left (7).toLower () == "http://")
			provider = Providers_ ["http"];
		else
		{
			emit error (tr ("Wrong mirror"));
			return;
		}

		if (!provider)
		{
			emit error (tr ("Could not find satisfying provider"));
			return;
		}

		if (!mirror.endsWith ('/'))
			mirror.append ('/');

		QFile file (QFileInfo (rep.Location_).fileName ());
		if (file.exists ())
		{
			if (!file.open (QIODevice::ReadOnly))
			{
				emit error ("Could not open the file");
				break;
			}
			QByteArray downloadedHash = QCryptographicHash::hash (file.readAll (), QCryptographicHash::Md5);
			if (downloadedHash.toHex () == rep.MD5_)
			{
				emit downloadedID (i);
				IDs2Download_.removeAt (i--);
				ToApply_ << rep;
				break;
			}
		}
		
		DirectDownloadParams ddp = { mirror + rep.URL_, QFileInfo (rep.Location_).fileName (), true, XmlSettingsManager::Instance ()->property ("SaveInHistory").toBool () };
		disconnect (this, SIGNAL (addDownload (DirectDownloadParams)), provider, 0);
		disconnect (provider, SIGNAL (jobAdded (int)), this, SLOT (fileDownloadAdded (int)));
		connect (this, SIGNAL (addDownload (DirectDownloadParams)), provider, SLOT (addDownload (DirectDownloadParams)));
		connect (provider, SIGNAL (jobAdded (int)), this, SLOT (fileDownloadAdded (int)));
		emit addDownload (ddp);
		qDebug () << Q_FUNC_INFO;

		DownloadWaiter_.first->lock ();
		DownloadWaiter_.second->wait (DownloadWaiter_.first);
		DownloadWaiter_.first->unlock ();

		if (!file.open (QIODevice::ReadOnly))
			continue;
		QByteArray downloadedHash = QCryptographicHash::hash (file.readAll (), QCryptographicHash::Md5);
		if (downloadedHash.toHex () != rep.MD5_)
		{
			qDebug () << "File not downloaded";
			file.close ();
			if (!file.remove ())
				return;
		}
		emit downloadedID (i);
		IDs2Download_.removeAt (i--);
		ToApply_ << rep;
	}

	if (IDs2Download_.size ())
	{
		DownloadState_ = DownloadError;
		emit error (tr ("Not all files were downloaded, sorry."));
		return;
	}
	DownloadState_ = DownloadedSuccessfully;
	emit finishedDownload ();
	ApplyUpdates ();
	*/
}

void Core::ApplyUpdates ()
{
	/*
	QSettings settings (Proxy::Instance ()->GetOrganizationName (), Proxy::Instance ()->GetApplicationName ());
	settings.beginGroup ("Updater");
	settings.beginGroup ("syncs");
	for (int i = 0; i < ToApply_.size (); ++i)
	{
		EntityRep rep = ToApply_.at (i);
		QString name = QFileInfo (rep.Location_).fileName ();
		if (!QFile::remove (rep.Location_))
			emit error (tr ("Removing old version failed."));
		if (!QFile::copy (name, rep.Location_))
			emit error (tr ("Copying failed."));
		if (!QFile::remove (name))
			emit error (tr ("Removing temporary file failed, do it yourself, cause I've done everything else."));

		settings.setValue (rep.Location_, static_cast<qulonglong> (rep.Build_));
	}
	settings.endGroup ();
	settings.endGroup ();

	ToApply_.clear ();
	emit finishedApplying ();
	*/
}

bool Core::Parse ()
{
	Entities_.clear ();
	ToApply_.clear ();

	QFile file (UpdateFilename_);
	if (!file.open (QIODevice::ReadOnly))
	{
		emit error (tr ("Could not open downloaded file."));
		return false;
	}
	QByteArray contents = file.readAll ();
	if (contents.isEmpty ())
	{
		emit error (tr ("Could not read downloaded file."));
		file.close ();
		return false;
	}
	file.close ();
	QFile::remove (UpdateFilename_);

	QDomDocument domdoc;
	int errorLine, errorColumn;
	QString errorMsg;
	if (!domdoc.setContent (contents, false, &errorMsg, &errorLine, &errorColumn))
	{
		emit error (tr ("Parse error %1 at line %2, column %3").arg (errorMsg).arg(errorLine).arg(errorColumn));
		return false;
	}

	QDomElement root = domdoc.documentElement ();
	if (root.tagName () != "lcupdate")
	{
		emit error (tr ("Downloaded file isn't LeechCraft Update file."));
		return false;
	}

	CollectFiles (root);

//	for (int i = 0; i < Files_.size (); ++i)
//		emit gotFile (i, Files_ [i].Name_, Files_ [i].Location_, Files_ [i].Size_, Files_ [i].Description_);

	return true;
}

void Core::CollectFiles (QDomElement &e)
{
	QDomElement fileChild = e.firstChildElement ("file");
	QSettings settings (Proxy::Instance ()->GetOrganizationName (), Proxy::Instance ()->GetApplicationName ());
	settings.beginGroup ("Updater");
	settings.beginGroup ("syncs");

	while (!fileChild.isNull ())
	{
		QDomElement name = fileChild.firstChildElement ("name");
		QDomElement descr = fileChild.firstChildElement ("desc");
		QDomElement url = fileChild.firstChildElement ("url");
		QDomElement loc = fileChild.firstChildElement ("location");
		QDomElement hash = fileChild.firstChildElement ("hash");
		QDomElement build = fileChild.firstChildElement ("build");
		QDomElement size = fileChild.firstChildElement ("size");
		if (name.isNull () || descr.isNull () || url.isNull () || loc.isNull () || hash.isNull () || build.isNull () || size.isNull ())
		{
			emit error (tr ("Malformed update file"));
			return;
		}

		QString md5 = hash.text ();
		QString location = loc.text ();

		if (build.text ().toULong () <= settings.value (location.trimmed (), 0).toULongLong ())
			continue;

		QFile file (QCoreApplication::applicationDirPath () + "/" + location);
		if (file.open (QIODevice::ReadOnly))
		{
			QByteArray localHash = QCryptographicHash::hash (file.readAll (), QCryptographicHash::Md5);
			if (localHash.toHex () != md5)
			{
//				FileRepresentation fr = {	md5.toAscii (),
//											location.trimmed (),
//											url.text ().trimmed (),
//											descr.text ().trimmed (),
//											name.text ().trimmed (),
//											build.text ().toULong (),
//											size.text ().toULong () };
//				Files_ << fr;
			}
		}

		fileChild = fileChild.nextSiblingElement ("file");
	}

	settings.endGroup ();
	settings.endGroup ();
}

bool Core::HandleSingleMirror (QObject *provider, const QString& mirror)
{
	QString tmpfn;
	{
		QTemporaryFile file ("lc.updater.update.xml.XXXXXX");
		file.open ();
		file.close ();
		tmpfn = QFileInfo (file).absoluteFilePath ();
	}
	UpdateFilename_ = tmpfn;
	DirectDownloadParams ddp = { mirror + QString ("update.xml"), tmpfn, true, false };
	disconnect (this, SIGNAL (addDownload (DirectDownloadParams)), provider, 0);
	disconnect (provider, SIGNAL (jobAdded (int)), this, SLOT (versionDownloadAdded (int)));
	connect (this, SIGNAL (addDownload (DirectDownloadParams)), provider, SLOT (addDownload (DirectDownloadParams)));
	connect (provider, SIGNAL (jobAdded (int)), this, SLOT (versionDownloadAdded (int)));
	emit addDownload (ddp);

	CheckWaiter_.first->lock ();
	CheckWaiter_.second->wait (CheckWaiter_.first);
	CheckWaiter_.first->unlock ();

	if (CheckState_ == CheckedSuccessfully)
		return true;
	else
	{
		QFile::remove (UpdateFilename_);
		UpdateFilename_ = QString ("");
		return false;
	}
}

