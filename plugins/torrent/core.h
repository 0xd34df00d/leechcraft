#ifndef CORE_H
#define CORE_H
#include <QObject>
#include <torrent_info.hpp>

class Core : public QObject
{
	Q_OBJECT
public:
	static Core* Instance ();
	Core (QObject *parent = 0);
	void Release ();
	libtorrent::torrent_info GetTorrentInfo (const QString&);
	libtorrent::torrent_info GetTorrentInfo (const QByteArray&);
signals:
	void error (QString);
};

#endif

