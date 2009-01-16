#ifndef TORRENTINFO_H
#define TORRENTINFO_H
#include <QTime>
#include <libtorrent/bitfield.hpp>
#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/torrent_info.hpp>

struct TorrentInfo
{
    QString Destination_,
            State_;
	libtorrent::torrent_status Status_;
	libtorrent::torrent_info Info_;
};

#endif

