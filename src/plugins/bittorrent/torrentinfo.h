#ifndef PLUGINS_BITTORRENT_TORRENTINFO_H
#define PLUGINS_BITTORRENT_TORRENTINFO_H
#include <memory>
#include <QTime>
#include <libtorrent/bitfield.hpp>
#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/torrent_info.hpp>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace BitTorrent
		{
			struct TorrentInfo
			{
				QString Destination_,
						State_;
				libtorrent::torrent_status Status_;
				std::auto_ptr<libtorrent::torrent_info> Info_;
			};
		};
	};
};

#endif

