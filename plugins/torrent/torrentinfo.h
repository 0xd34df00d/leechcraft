#ifndef TORRENTINFO_H
#define TORRENTINFO_H

struct TorrentInfo
{
	QString Tracker_,
			State_;
	quint64 Downloaded_,
			TotalSize_,
			FailedSize_,
			DHTNodesCount_,
			TotalPieces_,
			DownloadedPieces_;
	double	DownloadRate_,
			UploadRate_,
			Progress_;
	int ConnectedPeers_,
		ConnectedSeeds_,
		NextAnnounce_,
		AnnounceInterval_,
		PieceSize_;
};

#endif

