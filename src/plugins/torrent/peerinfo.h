#ifndef PEERINFO_H
#define PEERINFO_H
#include <vector>
#include <QTime>

struct PeerInfo
{
    QString IP_;
    quint64 DSpeed_, USpeed_;
    quint64 Downloaded_, Uploaded_;
    QString Client_;
    std::vector<bool> Pieces_;
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

