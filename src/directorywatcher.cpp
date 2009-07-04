#include "directorywatcher.h"
#include <QDir>
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

		settingsChanged ();

		connect (Watcher_.get (),
				SIGNAL (directoryChanged (const QString&)),
				this,
				SLOT (handleDirectoryChanged (const QString&)));
	}

	void DirectoryWatcher::settingsChanged ()
	{
		QStringList dirs = Watcher_->directories ();
		if (!dirs.isEmpty ())
		{
			Watcher_->removePaths (dirs);
			XmlSettingsManager::Instance ()->
				setProperty ("WatchedDirectoryOldContents", QStringList ());
		}

		QString path = XmlSettingsManager::Instance ()->
			property ("WatchDirectory").toString ();
		if (!path.isEmpty ())
		{
			Watcher_->addPath (path);
			handleDirectoryChanged (path);
		}
	}

	void DirectoryWatcher::handleDirectoryChanged (const QString& path)
	{
		QStringList old = XmlSettingsManager::Instance ()->
			property ("WatchedDirectoryOldContents").toStringList ();

		QDir dir (path);
		QStringList nl = dir.entryList ();
		XmlSettingsManager::Instance ()->
			setProperty ("WatchedDirectoryOldContents", nl);

		Q_FOREACH (QString oldStr, old)
			nl.removeAll (oldStr);

		Q_FOREACH (QString newStr, nl)
			emit gotEntity (Util::MakeEntity (dir.filePath (newStr).toUtf8 (), path, FromUserInitiated));
	}
};

