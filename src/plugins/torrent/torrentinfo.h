#ifndef TORRENTINFO_H
#define TORRENTINFO_H
#include <QTime>
#include <libtorrent/bitfield.hpp>

struct TorrentInfo
{
    QString Tracker_,
            State_;
    quint64 Downloaded_,
			WantedDownload_,
			UploadedTotal_,
            Uploaded_,
            TotalSize_,
			TorrentSize_,
            FailedSize_,
			RedundantBytes_,
            DHTNodesCount_,
            TotalPieces_,
            DownloadedPieces_,
			PieceSize_,
			BlockSize_,
			DownloadOverhead_,
			UploadOverhead_;
    double DownloadRate_,
           UploadRate_,
           Progress_,
           DistributedCopies_;
    int ConnectedPeers_,
        ConnectedSeeds_,
		PeersInList_,
		SeedsInList_,
		PeersInSwarm_,
		SeedsInSwarm_,
		ConnectCandidates_,
		UpBandwidthQueue_,
		DownBandwidthQueue_,
		LastScrape_,
		ActiveTime_,
		SeedingTime_,
		SeedRank_;
    QTime NextAnnounce_,
          AnnounceInterval_;
	libtorrent::bitfield Pieces_;
};

#endif

