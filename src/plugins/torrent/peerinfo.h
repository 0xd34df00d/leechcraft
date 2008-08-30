#ifndef PEERINFO_H
#define PEERINFO_H
#include <vector>
#include <QTime>
#include <libtorrent/bitfield.hpp>

struct PeerInfo
{
    QString IP_;
    quint64 DSpeed_, USpeed_;
    quint64 Downloaded_, Uploaded_;
    QString Client_;
	libtorrent::bitfield Pieces_;
    std::size_t LoadBalancing_;
    QTime LastActive_;
    int Hashfails_;
    int Failcount_;
    int DownloadingPiece_;
    int DownloadingBlock_;
    int DownloadingProgress_;
    int DownloadingTotal_;
};

#endif

