#ifndef TORRENTINFO_H
#define TORRENTINFO_H
#include <QTime>
#include <libtorrent/bitfield.hpp>

struct TorrentInfo
{
    QString Tracker_,
            State_;
    quint64 Downloaded_,
            Uploaded_,
            TotalSize_,
            FailedSize_,
            DHTNodesCount_,
            TotalPieces_,
            DownloadedPieces_;
    double DownloadRate_,
           UploadRate_,
           Progress_,
           DistributedCopies_;
    int ConnectedPeers_,
        ConnectedSeeds_,
        PieceSize_;
    QTime NextAnnounce_,
          AnnounceInterval_;
	libtorrent::bitfield Pieces_;
};

#endif

