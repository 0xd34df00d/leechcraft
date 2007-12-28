#ifndef CORE_H
#define CORE_H
#include <QAbstractItemModel>
#include <QMap>
#include <QList>
#include <torrent_info.hpp>
#include <torrent_handle.hpp>
#include <session.hpp>

class Core : public QAbstractItemModel
{
	Q_OBJECT

public:
	typedef ulong TorrentID_t;
private:
	libtorrent::session *Session_;
	QMap<TorrentID_t, libtorrent::torrent_handle> Handles_;
	QList<QString> Headers_;
	TorrentID_t CurrentID_;
public:
	static Core* Instance ();
	Core (QObject *parent = 0);
	void Release ();

	virtual int columnCount (const QModelIndex&) const;
	virtual QVariant data (const QModelIndex&, int role = Qt::DisplayRole) const;
	virtual Qt::ItemFlags flags (const QModelIndex&) const;
	virtual bool hasChildren (const QModelIndex&) const;
	virtual QVariant headerData (int, Qt::Orientation, int role = Qt::DisplayRole) const;
	virtual QModelIndex index (int, int, const QModelIndex& parent = QModelIndex ()) const;
	virtual QModelIndex parent (const QModelIndex&) const;
	virtual int rowCount (const QModelIndex& parent = QModelIndex ()) const;

	libtorrent::torrent_info GetTorrentInfo (const QString&);
	libtorrent::torrent_info GetTorrentInfo (const QByteArray&);
	void AddFile (const QString&, const QString&);
	void RemoveTorrent (TorrentID_t);
signals:
	void error (QString);
	void torrentAdded (TorrentID_t);
	void torrentRemoved (TorrentID_t);
};

#endif

