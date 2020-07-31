/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "gpoddb.h"
#include <QDir>
#include <QFutureWatcher>
#include <QtConcurrentRun>
#include <QtDebug>
#include <interfaces/lmp/iunmountablesync.h>
#include <gpod/itdb.h>

namespace LC
{
namespace LMP
{
namespace jOS
{
	GpodDb::GpodDb (const QString& path, QObject *parent)
	: QObject { parent }
	, LocalPath_ { path }
	{
		qDebug () << Q_FUNC_INFO << path;
	}

	GpodDb::~GpodDb ()
	{
		itdb_free (DB_);
	}

	GpodDb::InitResult GpodDb::Reinitialize ()
	{
		const auto& nativePath = QDir::toNativeSeparators (LocalPath_).toUtf8 ();

		GError *gerr = nullptr;
		itdb_init_ipod (nativePath, nullptr, "leechpod", &gerr);

		if (gerr)
		{
			const auto& msg = QString::fromUtf8 (gerr->message);
			g_error_free (gerr);
			return { Result::OtherError, msg };
		}
		else
			return { Result::Success, {} };
	}

	GpodDb::InitResult GpodDb::Load ()
	{
		const auto& nativePath = QDir::toNativeSeparators (LocalPath_).toUtf8 ();
		GError *gerr = nullptr;
		if ((DB_ = itdb_parse (nativePath, &gerr)))
			return { Result::Success, {} };

		if (gerr && gerr->domain == ITDB_FILE_ERROR && gerr->code == ITDB_FILE_ERROR_NOTFOUND)
		{
			g_error_free (gerr);
			return { Result::NotFound, {} };
		}

		QString text;
		if (gerr)
		{
			text = tr ("Error loading iTunes database: %1.")
					.arg (QString::fromUtf8 (gerr->message));
			g_error_free (gerr);
		}
		else
			text = tr ("Error loading iTunes database.");

		return { Result::OtherError, text };
	}

	bool GpodDb::Save () const
	{
		qDebug () << Q_FUNC_INFO;
		GError *gerr = nullptr;
		itdb_write (DB_, &gerr);

		if (!gerr)
			return true;

		qDebug () << Q_FUNC_INFO
				<< gerr->message;
		g_error_free (gerr);
		return false;
	}

	Itdb_Track* GpodDb::AddTrack (const QString& path, const QString& filename, const UnmountableFileInfo& info)
	{
		auto dup = [] (const QString& str) { return strdup (str.toUtf8 ().constData ()); };

		const auto track = itdb_track_new ();
		track->track_nr = info.TrackNumber_;
		track->title = dup (info.TrackTitle_);
		track->artist = dup (info.Artist_);
		track->album = dup (info.Album_);
		track->year = info.AlbumYear_;
		track->genre = dup (info.Genres_.join (" / "));
		track->size = QFileInfo { path }.size ();

		track->transferred = 1;

		track->filetype = strdup ("mp3");

		const auto& suffix = filename.section ('.', -1, -1).toUpper ();
		track->filetype_marker = 0;
		for (int i = 0; i < 4; ++i)
		{
			track->filetype_marker <<= 8;
			track->filetype_marker |= (i >= suffix.length () ?
						' ' :
						suffix [i].toLatin1 ());
		}

		track->ipod_path = strdup (filename.toUtf8 ().mid (1));
		itdb_filename_fs2ipod (track->ipod_path);

		itdb_track_add (DB_, track, -1);

		itdb_playlist_add_track (itdb_playlist_mpl (DB_), track, -1);

		return track;
	}

	std::shared_ptr<void> GpodDb::GetSyncGuard () const
	{
		itdb_start_sync (DB_);
		return std::shared_ptr<void> (nullptr, [this] (void*) { itdb_stop_sync (DB_); });
	}
}
}
}
