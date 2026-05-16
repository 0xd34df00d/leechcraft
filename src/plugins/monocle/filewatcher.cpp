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
	struct FileWatcher::FileIdentity
	{
		qint64 Size_;
		QDateTime LastModified_;

		bool operator== (const FileIdentity&) const = default;

		friend QDebug operator<< (QDebug dbg, const FileIdentity& id)
		{
			return dbg << std::tie (id.Size_, id.LastModified_);
		}
	};

	FileWatcher::MaybeFileIdentity FileWatcher::MakeIdentity (const QString& path)
	{
		const QFileInfo fi { path };
		if (!fi.exists ())
			return {};

		struct stat st;
		if (stat (QFile::encodeName (path).constData (), &st) != 0)
			return {};

		return FileIdentity
		{
			.Size_ = fi.size (),
			.LastModified_ = fi.lastModified (),
		};
	}

	FileWatcher::FileWatcher (QObject *parent)
	: QObject { parent }
	, LastIdentityHolder_ { std::make_unique<MaybeFileIdentity> () }
	, LastIdentity_ { *LastIdentityHolder_ }
	{
		connect (&Watcher_,
				&QFileSystemWatcher::directoryChanged,
				this,
				&FileWatcher::CheckReload);
		connect (&Watcher_,
				&QFileSystemWatcher::fileChanged,
				this,
				[this]
				{
					if (!Watcher_.files ().contains (CurrentFile_))
						Watcher_.addPath (CurrentFile_);
					CheckReload ();
				});

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

	FileWatcher::~FileWatcher () = default;

	void FileWatcher::SetWatchedFile (const QString& file)
	{
		if (CurrentFile_ == file)
			return;

		CurrentFile_ = file;
		LastIdentity_ = MakeIdentity (CurrentFile_);

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
