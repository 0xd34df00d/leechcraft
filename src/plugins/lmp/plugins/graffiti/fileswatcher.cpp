/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "fileswatcher.h"
#include <QFileSystemWatcher>
#include <QSet>
#include <QStringList>

namespace LeechCraft
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
