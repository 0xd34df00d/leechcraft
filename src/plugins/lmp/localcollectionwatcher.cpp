/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "localcollectionwatcher.h"
#include <algorithm>
#include <QTimer>
#include "core.h"
#include "localcollection.h"
#include "recursivedirwatcher.h"

namespace LC
{
namespace LMP
{
	LocalCollectionWatcher::LocalCollectionWatcher (QObject *parent)
	: QObject (parent)
	, Watcher_ (new RecursiveDirWatcher (this))
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

	void LocalCollectionWatcher::AddPath (const QString& path)
	{
		Watcher_->AddRoot (path);
	}

	void LocalCollectionWatcher::RemovePath (const QString& path)
	{
		Watcher_->RemoveRoot (path);
	}

	void LocalCollectionWatcher::ScheduleDir (const QString& dir)
	{
		if (ScanTimer_->isActive ())
			ScanTimer_->stop ();
		ScanTimer_->start (5000);

		if (std::any_of (ScheduledDirs_.begin (), ScheduledDirs_.end (),
				[&dir] (const QString& other) { return dir.startsWith (other); }))
			return;

		const auto pos = std::remove_if (ScheduledDirs_.begin (), ScheduledDirs_.end (),
				[&dir] (const QString& other) { return other.startsWith (dir); });
		ScheduledDirs_.erase (pos, ScheduledDirs_.end ());

		ScheduledDirs_ << dir;
	}

	void LocalCollectionWatcher::handleDirectoryChanged (const QString& path)
	{
		ScheduleDir (path);
	}

	void LocalCollectionWatcher::rescanQueue ()
	{
		for (const auto& path : ScheduledDirs_)
			Core::Instance ().GetLocalCollection ()->Scan (path, false);

		ScheduledDirs_.clear ();
	}
}
}
