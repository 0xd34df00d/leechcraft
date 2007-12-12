#include <QtCore>
#include <QtXml>
#include <QtDebug>
#include <interfaces/interfaces.h>
#include "settingsmanager.h"
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

void Core::downloadUpdates ()
{
	if (IsChecking () || IsDownloading ())
		return;

	DownloadState_ = ShouldDownload;
	Waiter_.second->wakeOne ();
}

void Core::handleDownloadFinished (int id)
{
	if (id == UpdateInfoID_)
	{
		CheckState_ = CheckedSuccessfully;
		CheckWaiter_.second->wakeAll ();
	}
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
	QStringList mirrors = SettingsManager::Instance ()->GetMirrors ();
	bool result = false;
	for (int i = 0; i < mirrors.size (); ++i)
	{
		IDirectDownload *idd;
		QString mirror = mirrors [i];
		if (mirror.left (6).toLower () == "ftp://")
			idd = qobject_cast<IDirectDownload*> (Providers_ ["ftp"]);
		else if (mirror.left (7).toLower () == "http://")
			idd = qobject_cast<IDirectDownload*> (Providers_ ["http"]);
		else
			continue;

		if (!idd)
			continue;

		if (!mirror.endsWith ('/'))
			mirror.append ('/');

		if ((result = HandleSingleMirror (idd, mirror)))
			break;
	}

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
}

bool Core::Parse ()
{
	Files_.clear ();

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

	for (int i = 0; i < Files_.size (); ++i)
		emit gotFile (Files_ [i].Name_, Files_ [i].Location_, Files_ [i].Size_, Files_ [i].Description_);

	return true;
}

void Core::CollectFiles (QDomElement &e)
{
	QDomElement fileChild = e.firstChildElement ("file");
	while (!fileChild.isNull ())
	{
		QDomElement name = fileChild.firstChildElement ("name");
		QDomElement descr = fileChild.firstChildElement ("desc");
		QDomElement url = fileChild.firstChildElement ("url");
		QDomElement loc = fileChild.firstChildElement ("location");
		QDomElement hash = fileChild.firstChildElement ("hash");
		QDomElement size = fileChild.firstChildElement ("size");
		if (name.isNull () || descr.isNull () || url.isNull () || loc.isNull () || hash.isNull () || size.isNull ())
		{
			emit error (tr ("Malformed update file"));
			return;
		}

		QString md5 = hash.text ();
		QString location = loc.text ();

		QFile file (QCoreApplication::applicationDirPath () + "/" + location);
		if (file.open (QIODevice::ReadOnly))
		{
			QByteArray localHash = QCryptographicHash::hash (file.readAll (), QCryptographicHash::Md5);
			if (localHash != md5)
			{
				FileRepresentation fr = { md5.toAscii (), location.trimmed (), url.text ().trimmed (), descr.text ().trimmed (), name.text ().trimmed () , size.text ().toULong () };
				Files_ << fr;
			}
		}

		fileChild = fileChild.nextSiblingElement ("file");
	}
}

bool Core::HandleSingleMirror (IDirectDownload *idd, const QString& mirror)
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
	UpdateInfoID_ = idd->AddDownload (ddp);

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

