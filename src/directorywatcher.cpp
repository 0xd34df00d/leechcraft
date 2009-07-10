#include "directorywatcher.h"
#include <QDir>
#include <QTimer>
#include <plugininterface/util.h>
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
	DirectoryWatcher::DirectoryWatcher (QObject *parent)
	: QObject (parent)
	, Watcher_ (new QFileSystemWatcher)
	{
		XmlSettingsManager::Instance ()->RegisterObject ("WatchDirectory",
				this,
				"settingsChanged");

		QTimer::singleShot (5000,
				this,
				SLOT (settingsChanged ()));

		connect (Watcher_.get (),
				SIGNAL (directoryChanged (const QString&)),
				this,
				SLOT (handleDirectoryChanged (const QString&)));
	}

	void DirectoryWatcher::settingsChanged ()
	{
		QString path = XmlSettingsManager::Instance ()->
			property ("WatchDirectory").toString ();
		QStringList dirs = Watcher_->directories ();
		if (dirs.size () == 1 && 
				dirs.at (0) == path)
			return;

		if (!dirs.isEmpty ())
		{
			Watcher_->removePaths (dirs);
			XmlSettingsManager::Instance ()->
				setProperty ("WatchedDirectoryOldContents", QStringList ());
		}

		if (!path.isEmpty ())
		{
			Watcher_->addPath (path);
			handleDirectoryChanged (path);
		}
	}

	void DirectoryWatcher::handleDirectoryChanged (const QString& path)
	{
		qDebug () << Q_FUNC_INFO;
		QStringList old;
		if (Olds_.isEmpty ())
			old = XmlSettingsManager::Instance ()->
				property ("WatchedDirectoryOldContents").toStringList ();

		QDir dir (path);
		QList<QFileInfo> nl = dir.entryInfoList ();
		QStringList nls;
		Q_FOREACH (QFileInfo fi, nl)
			nls << fi.fileName ();
		XmlSettingsManager::Instance ()->
			setProperty ("WatchedDirectoryOldContents", nls);

		if (Olds_.isEmpty ())
			Q_FOREACH (QString oldStr, old)
				Q_FOREACH (QFileInfo fi, nl)
					if (fi.fileName () == oldStr)
					{
						nl.removeAll (fi);
						break;
					}
		else
			Q_FOREACH (QFileInfo oldFi, Olds_)
				nl.removeAll (oldFi);

		Olds_ = nl;

		Q_FOREACH (QFileInfo newFi, nl)
			emit gotEntity (Util::MakeEntity (newFi.absoluteFilePath ().toUtf8 (),
						path, FromUserInitiated));
	}
};

