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

#include "localcollectionwatcher.h"
#include <QFileSystemWatcher>
#include <QtDebug>
#include <QDir>
#include "core.h"
#include "localcollection.h"
#include "util.h"

namespace LeechCraft
{
namespace LMP
{
	LocalCollectionWatcher::LocalCollectionWatcher (QObject *parent)
	: QObject (parent)
	, Watcher_ (new QFileSystemWatcher (this))
	{
		connect (Watcher_,
				SIGNAL (directoryChanged (QString)),
				this,
				SLOT (handleDirectoryChanged (QString)));
	}

	namespace
	{
		QStringList CollectSubdirs (const QString& path)
		{
			QDir dir (path);
			const auto& list = dir.entryList (QDir::Dirs | QDir::NoDotAndDotDot);

			QStringList result (path);
			std::for_each (list.begin (), list.end (),
					[&dir, &result] (decltype (list.front ()) item)
						{ result += CollectSubdirs (dir.filePath (item)); });
			return result;
		}
	}

	void LocalCollectionWatcher::AddPath (const QString& path)
	{
		const auto& paths = CollectSubdirs (path);
		Dir2Subdirs_ [path] = paths;
		Watcher_->addPaths (paths);
	}

	void LocalCollectionWatcher::RemovePath (const QString& path)
	{
		Watcher_->removePaths (Dir2Subdirs_ [path]);
	}

	void LocalCollectionWatcher::handleDirectoryChanged (const QString& path)
	{
		Core::Instance ().GetLocalCollection ()->Scan (path, false);
	}
}
}
