/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#include "filewatcher.h"
#include <QTimer>
#include "documenttab.h"

namespace LeechCraft
{
namespace Monocle
{
	FileWatcher::FileWatcher (DocumentTab *tab)
	: QObject (tab)
	, Tab_ (tab)
	, IsScheduled_ (false)
	{
		connect (tab,
				SIGNAL (fileLoaded (QString)),
				this,
				SLOT (setWatched (QString)));

		connect (&Watcher_,
				SIGNAL (fileChanged (QString)),
				this,
				SLOT (handleFileChanged (QString)));
	}

	void FileWatcher::handleFileChanged (const QString& path)
	{
		if (IsScheduled_ || path != CurrentFile_)
			return;

		QTimer::singleShot (2000,
				this,
				SLOT (doReload ()));
		IsScheduled_ = true;
	}

	void FileWatcher::doReload ()
	{
		Tab_->ReloadDoc (CurrentFile_);
		IsScheduled_ = false;
	}

	void FileWatcher::setWatched (const QString& file)
	{
		if (CurrentFile_ == file)
			return;

		CurrentFile_ = file;
		Watcher_.removePaths (Watcher_.files ());

		Watcher_.addPath (file);
	}
}
}
