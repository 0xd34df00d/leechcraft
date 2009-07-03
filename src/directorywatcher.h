#ifndef DIRECTORYWATCHER_H
#define DIRECTORYWATCHER_H
#include <memory>
#include <QObject>
#include <QFileSystemWatcher>
#include "interfaces/structures.h"

namespace LeechCraft
{
	class DirectoryWatcher : public QObject
	{
		Q_OBJECT

		std::auto_ptr<QFileSystemWatcher> Watcher_;
	public:
		DirectoryWatcher (QObject* = 0);
	private slots:
		void settingsChanged ();
		void handleDirectoryChanged (const QString&);
	signals:
		void gotEntity (const LeechCraft::DownloadEntity&);
	};
};

#endif

