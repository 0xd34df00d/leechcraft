#ifndef PEERINFO_H
#define PEERINFO_H
#include <vector>

struct PeerInfo
{
    QString IP_;
    bool Seed_;
    quint64 DSpeed_, USpeed_;
    quint64 Downloaded_, Uploaded_;
    QString Client_;
    std::vector<bool> Pieces_;
};

#endif

