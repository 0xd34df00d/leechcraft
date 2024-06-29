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
#include <QtDebug>
#include "documenttab.h"
#include "pagegraphicsitem.h"

namespace LC::Monocle
{
	namespace
	{
		auto MakeIdentity (const QString& path)
		{
			const QFileInfo fi { path };
			return FileWatcher::FileIdentity_t { fi.size (), fi.lastModified () };
		}
	}

	FileWatcher::FileWatcher (DocumentTab& tab)
	: QObject { &tab }
	, Tab_ { tab }
	{
		connect (&tab,
				&DocumentTab::fileLoaded,
				this,
				&FileWatcher::SetWatchedFile);

		connect (&Watcher_,
				&QFileSystemWatcher::directoryChanged,
				this,
				&FileWatcher::CheckReload);
		connect (&Watcher_,
				&QFileSystemWatcher::fileChanged,
				this,
				&FileWatcher::CheckReload);

		using namespace std::chrono_literals;
		constexpr auto ReloadTimerInterval = 750ms;

		ReloadTimer_.setSingleShot (true);
		ReloadTimer_.setInterval (ReloadTimerInterval);
		ReloadTimer_.callOnTimeout ([this]
				{
					Tab_.ReloadDoc (CurrentFile_);
					LastIdentity_ = MakeIdentity (CurrentFile_);
				});
	}

	void FileWatcher::SetWatchedFile (const QString& file)
	{
		if (CurrentFile_ == file)
			return;

		CurrentFile_ = file;

		if (const auto& existing = Watcher_.directories () + Watcher_.files ();
			!existing.isEmpty ())
			Watcher_.removePaths (existing);

		const auto failed = Watcher_.addPaths ({ CurrentFile_, QFileInfo { CurrentFile_ }.dir ().path () });
		if (!failed.isEmpty ())
			qWarning () << "failed to watch" << failed;
	}

	void FileWatcher::CheckReload ()
	{
		const auto& newIdentity = MakeIdentity (CurrentFile_);
		if (LastIdentity_ == newIdentity)
			return;

		LastIdentity_ = newIdentity;
		ReloadTimer_.start ();
	}
}
