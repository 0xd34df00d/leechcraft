#ifndef PLUGINS_BITTORRENT_PEERINFO_H
#define PLUGINS_BITTORRENT_PEERINFO_H
#include <vector>
#include <boost/shared_ptr.hpp>
#include <QTime>
#include <libtorrent/peer_info.hpp>
#include <libtorrent/bitfield.hpp>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace BitTorrent
		{
			struct PeerInfo
			{
				QString IP_;
				QString Client_;
				int RemoteHas_;
				boost::shared_ptr<libtorrent::peer_info> PI_;
			};
		};
	};
};

#endif

