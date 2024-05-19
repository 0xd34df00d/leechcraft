/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "filewatcher.h"
#include <QTimer>
#include <QFileInfo>
#include "documenttab.h"
#include "pagegraphicsitem.h"

namespace LC
{
namespace Monocle
{
	FileWatcher::FileWatcher (DocumentTab *tab)
	: QObject (tab)
	, Tab_ (tab)
	{
		connect (tab,
				&DocumentTab::fileLoaded,
				this,
				&FileWatcher::SetWatched);

		connect (&Watcher_,
				&QFileSystemWatcher::directoryChanged,
				this,
				&FileWatcher::CheckReload);
		connect (&Watcher_,
				&QFileSystemWatcher::fileChanged,
				this,
				&FileWatcher::CheckReload);

		connect (&ReloadTimer_,
				&QTimer::timeout,
				this,
				&FileWatcher::DoReload);
		ReloadTimer_.setSingleShot (true);
		ReloadTimer_.setInterval (750);
	}

	namespace
	{
		auto MakeIdentity (const QString& path)
		{
			const QFileInfo fi { path };
			return FileWatcher::FileIdentity_t { fi.size (), fi.lastModified () };
		}
	}

	void FileWatcher::ResetWatcher ()
	{
		const auto& existing = Watcher_.directories () + Watcher_.files ();
		if (!existing.isEmpty ())
			Watcher_.removePaths (existing);

		Watcher_.addPath (CurrentFile_);
		Watcher_.addPath (QFileInfo { CurrentFile_ }.dir ().path ());

		LastIdentity_ = MakeIdentity (CurrentFile_);
	}

	void FileWatcher::CheckReload ()
	{
		const auto& newIdentity = MakeIdentity (CurrentFile_);
		if (LastIdentity_ == newIdentity)
			return;

		LastIdentity_ = newIdentity;

		ReloadTimer_.start ();
	}

	void FileWatcher::DoReload ()
	{
		Tab_->ReloadDoc (CurrentFile_);
		ResetWatcher ();
	}

	void FileWatcher::SetWatched (const QString& file)
	{
		if (CurrentFile_ == file)
			return;

		CurrentFile_ = file;
		ResetWatcher ();
	}
}
}
