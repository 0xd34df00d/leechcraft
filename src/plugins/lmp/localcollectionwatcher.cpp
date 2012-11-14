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
#include <algorithm>
#include <QtConcurrentRun>
#include <QFileSystemWatcher>
#include <QtDebug>
#include <QDir>
#include <QTimer>
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
	, ScanTimer_ (new QTimer (this))
	{
		connect (Watcher_,
				SIGNAL (directoryChanged (QString)),
				this,
				SLOT (handleDirectoryChanged (QString)));

		ScanTimer_->setSingleShot (true);
		connect (ScanTimer_,
				SIGNAL (timeout ()),
				this,
				SLOT (rescanQueue ()));
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
		qDebug () << Q_FUNC_INFO << "scanning" << path;
		auto watcher = new QFutureWatcher<QStringList> ();
		watcher->setProperty ("Path", path);
		connect (watcher,
				SIGNAL (finished ()),
				this,
				SLOT (handleSubdirsCollected ()));

		watcher->setFuture (QtConcurrent::run (CollectSubdirs, path));
	}

	void LocalCollectionWatcher::RemovePath (const QString& path)
	{
		Watcher_->removePaths (Dir2Subdirs_ [path]);
	}

	void LocalCollectionWatcher::ScheduleDir (const QString& dir)
	{
		if (ScanTimer_->isActive ())
			ScanTimer_->stop ();
		ScanTimer_->start (2000);

		if (!ScheduledDirs_.contains (dir))
			ScheduledDirs_ << dir;
	}

	void LocalCollectionWatcher::handleSubdirsCollected ()
	{
		auto watcher = dynamic_cast<QFutureWatcher<QStringList>*> (sender ());
		if (!watcher)
			return;

		watcher->deleteLater ();

		const auto& paths = watcher->result ();
		const auto& path = watcher->property ("Path").toString ();
		Dir2Subdirs_ [path] = paths;
		Watcher_->addPaths (paths);
	}

	void LocalCollectionWatcher::handleDirectoryChanged (const QString& path)
	{
		ScheduleDir (path);
	}

	void LocalCollectionWatcher::rescanQueue ()
	{
		Q_FOREACH (const auto& path, ScheduledDirs_)
			Core::Instance ().GetLocalCollection ()->Scan (path, false);

		ScheduledDirs_.clear ();
	}
}
}
