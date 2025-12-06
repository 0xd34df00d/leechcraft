/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "filewatcher.h"
#include <QDir>
#include <QTimer>
#include <QFileInfo>
#include <QtDebug>

namespace LC::Monocle
{
	namespace
	{
		std::optional<FileWatcher::FileIdentity_t> MakeIdentity (const QString& path)
		{
			const QFileInfo fi { path };
			if (!fi.exists ())
				return {};

			return FileWatcher::FileIdentity_t { fi.size (), fi.lastModified () };
		}
	}

	FileWatcher::FileWatcher (QObject *parent)
	: QObject { parent }
	{
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
					emit reloadNeeded (CurrentFile_);
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
		if (!newIdentity || LastIdentity_ == newIdentity)
			return;

		LastIdentity_ = newIdentity;
		ReloadTimer_.start ();
	}
}
