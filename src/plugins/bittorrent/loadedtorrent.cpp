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
#include "ltutils.h"

namespace LC::BitTorrent
{
	namespace
	{
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
						<< QString::fromStdString (loaded.ti->name ())
						<< e.what ();
				return loaded;
			}

			resumed.ti = std::move (loaded.ti);
			resumed.dht_nodes = std::move (loaded.dht_nodes);
			return resumed;
		}
	}

	LoadedTorrent LoadedTorrent::Load (const QByteArray& torrent, const QByteArray& resumeData)
	{
		auto loaded = libtorrent::load_torrent_buffer (libtorrent::span { torrent.constData (), torrent.size () });
		auto creationInfo = GetTorrentCreationInfo (loaded);
		return { MergeResumeData (std::move (loaded), resumeData), std::move (creationInfo) };
	}
}
