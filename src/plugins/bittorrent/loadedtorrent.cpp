/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "loadedtorrent.h"
#include <QByteArray>
#include <QString>
#include <QtDebug>
#include <libtorrent/load_torrent.hpp>
#include <libtorrent/read_resume_data.hpp>
#include <libtorrent/torrent_info.hpp>
#include <libtorrent/version.hpp>
#include "ltutils.h"

namespace LC::BitTorrent
{
	namespace
	{
		TorrentCreationInfo GetTorrentCreationInfo (const libtorrent::add_torrent_params& params)
		{
#if LIBTORRENT_VERSION_NUM >= 20100
			const auto& creator = params.created_by;
			const auto& comment = params.comment;
			const auto creationDate = params.creation_date;
#else
			const auto& info = *params.ti;
			const auto& creator = info.creator ();
			const auto& comment = info.comment ();
			const auto creationDate = info.creation_date ();
#endif
			return
			{
				.Creator_ = QString::fromStdString (creator),
				.Comment_ = QString::fromStdString (comment),
				.CreationDate_ = creationDate ? QDateTime::fromSecsSinceEpoch (creationDate) : QDateTime {},
				.DHTNodes_ = static_cast<int> (params.dht_nodes.size ()),
			};
		}

		libtorrent::add_torrent_params MergeResumeData (libtorrent::add_torrent_params loaded, const QByteArray& resumeData)
		{
			if (resumeData.isEmpty ())
				return loaded;

			libtorrent::add_torrent_params resumed;
			try
			{
				resumed = libtorrent::read_resume_data (libtorrent::span { resumeData.constData (), resumeData.size () });
			}
			catch (const std::exception& e)
			{
				qWarning () << "ignoring unreadable resume data for"
						<< QString::fromStdString (loaded.ti ? loaded.ti->name () : loaded.name)
						<< e.what ();
				return loaded;
			}

			if (loaded.ti)
				resumed.ti = std::move (loaded.ti);
			resumed.dht_nodes = std::move (loaded.dht_nodes);
			return resumed;
		}
	}

	LoadedTorrent LoadedTorrent::Load (const QByteArray& torrent, const QByteArray& resumeData)
	{
		auto loaded = libtorrent::load_torrent_buffer (libtorrent::span { torrent.constData (), torrent.size () });
		// TODO drop along with libtorrent < 2.1 support: 2.0 describes a torrent file carrying only
		// a magnet link as a `torrent_info` without metadata, where 2.1 leaves `ti` empty.
		if (loaded.ti && !loaded.ti->is_valid ())
			loaded.ti.reset ();

		auto creationInfo = loaded.ti ? GetTorrentCreationInfo (loaded) : TorrentCreationInfo {};
		return { MergeResumeData (std::move (loaded), resumeData), std::move (creationInfo) };
	}

	std::string LoadedTorrent::FilePath (libtorrent::file_index_t index) const
	{
		const auto renamed = Params_.renamed_files.find (index);
		return renamed != Params_.renamed_files.end () ?
				renamed->second :
				GetOriginalFileLayout (*Params_.ti).file_path (index);
	}
}
