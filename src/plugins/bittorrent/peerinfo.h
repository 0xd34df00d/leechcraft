#ifndef PLUGINS_BITTORRENT_PEERINFO_H
#define PLUGINS_BITTORRENT_PEERINFO_H
#include <vector>
#include <QTime>
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
				quint64 DSpeed_,
						USpeed_;
				quint64 Downloaded_,
						Uploaded_;
				QString Client_;
				int NumPieces_;
				int RemoteHas_;
				QTime LastActive_;
				int Hashfails_;
				int Failcount_;
				int DownloadingPiece_;
				int DownloadingBlock_;
				int DownloadingProgress_;
				int DownloadingTotal_;
			};
		};
	};
};

#endif

