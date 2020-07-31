/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "fileswatcher.h"
#include <QFileSystemWatcher>
#include <QSet>
#include <QStringList>

namespace LC
{
namespace LMP
{
namespace Graffiti
{
	FilesWatcher::FilesWatcher (QObject *parent)
	: QObject (parent)
	, Watcher_ (new QFileSystemWatcher (this))
	{
		connect (Watcher_,
				SIGNAL (directoryChanged (QString)),
				this,
				SIGNAL (rereadFiles ()));
	}

	void FilesWatcher::Clear ()
	{
		const auto& dirs = Watcher_->directories ();
		if (!dirs.isEmpty ())
			Watcher_->removePaths (dirs);
	}

	void FilesWatcher::AddFiles (const QList<QFileInfo>& infos)
	{
		QSet<QString> existingDirs;
		for (const auto& info : infos)
		{
			const auto& dir = info.absolutePath ();
			if (existingDirs.contains (dir))
				continue;

			existingDirs << dir;
			Watcher_->addPath (dir);
		}
	}
}
}
}
