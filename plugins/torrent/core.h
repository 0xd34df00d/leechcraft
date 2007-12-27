#ifndef CORE_H
#define CORE_H
#include <QObject>
#include <QMap>
#include <torrent_info.hpp>
#include <torrent_handle.hpp>
#include <session.hpp>

class Core : public QObject
{
	Q_OBJECT

public:
	typedef ulong TorrentID_t;
private:
	libtorrent::session *Session_;
	QMap<TorrentID_t, libtorrent::torrent_handle> Handles_;
	TorrentID_t CurrentID_;
public:
	static Core* Instance ();
	Core (QObject *parent = 0);
	void Release ();
	libtorrent::torrent_info GetTorrentInfo (const QString&);
	libtorrent::torrent_info GetTorrentInfo (const QByteArray&);
	void AddFile (const QString&, const QString&);
	void RemoveTorrent (TorrentID_t);
signals:
	void error (QString);
	void torrentAdded (TorrentID_t);
};

#endif

