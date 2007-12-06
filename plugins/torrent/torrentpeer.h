#ifndef TORRENTPEER_H
#define TORRENTPEER_H
#include <QHostAddress>
#include <QString>
#include <QBitArray>

/*! @brief Represents remote peer.
 *
 */
struct TorrentPeer
{
	QHostAddress Address_;
	quint16 Port_;
	bool Interesting_;
	bool Seed_;
	quint32 LastVisited_, ConnectStart_, ConnectTime_;
	QString ID_;
	QBitArray Pieces_;
	int NumComplete_;
};

bool operator== (const TorrentPeer& tp1, const TorrentPeer& tp2);

#endif

