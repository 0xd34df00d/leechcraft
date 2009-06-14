#ifndef PLUGINS_BITTORRENT_NEWTORRENTPARAMS_H
#define PLUGINS_BITTORRENT_NEWTORRENTPARAMS_H
#include <QDate>
#include <QString>
#include <QStringList>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace BitTorrent
		{
			struct NewTorrentParams
			{
				QString OutputDirectory_
					, TorrentName_
					, AnnounceURL_
					, Comment_
					, Path_;
				QDate Date_;
				int PieceSize_;
				QStringList URLSeeds_;
				QStringList DHTNodes_;
				bool DHTEnabled_;
			};
		};
	};
};

#endif

