/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "filewatcher.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTimer>
#include <QtDebug>
#ifdef Q_OS_UNIX
#include <sys/stat.h>
#endif

namespace LC::Monocle
{
	struct FileWatcher::FileIdentity
	{
		qint64 Size_;
		QDateTime LastModified_;
#ifdef Q_OS_UNIX
		ino_t Inode_;
#else
		bool Inode_ = false; // dummy
#endif

		bool operator== (const FileIdentity&) const = default;

		friend QDebug operator<< (QDebug dbg, const FileIdentity& id)
		{
			return dbg << std::tie (id.Size_, id.LastModified_, id.Inode_);
		}
	};

	FileWatcher::MaybeFileIdentity FileWatcher::MakeIdentity (const QString& path)
	{
		const QFileInfo fi { path };
		if (!fi.exists ())
			return {};

#ifdef Q_OS_UNIX
		struct stat st;
		if (stat (QFile::encodeName (path).constData (), &st) != 0)
			return {};
#endif

		return FileIdentity
		{
			.Size_ = fi.size (),
			.LastModified_ = fi.lastModified (),
#ifdef Q_OS_UNIX
			.Inode_ = st.st_ino,
#endif
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
				&FileWatcher::HandleChange);
		connect (&Watcher_,
				&QFileSystemWatcher::fileChanged,
				this,
				&FileWatcher::HandleChange);

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

	void FileWatcher::HandleChange ()
	{
		const auto newIdentity = MakeIdentity (CurrentFile_);
		if (!newIdentity || LastIdentity_ == newIdentity)
			return;

		if (!LastIdentity_ || LastIdentity_->Inode_ != newIdentity->Inode_)
		{
			Watcher_.removePath (CurrentFile_);
			if (!Watcher_.addPath (CurrentFile_))
				qWarning () << "failed to rearm watch on" << CurrentFile_;
		}

		LastIdentity_ = newIdentity;
		ReloadTimer_.start ();
	}
}
