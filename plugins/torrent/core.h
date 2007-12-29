#ifndef CORE_H
#define CORE_H
#include <QAbstractItemModel>
#include <QPair>
#include <QList>
#include <torrent_info.hpp>
#include <torrent_handle.hpp>
#include <session.hpp>
#include "torrentinfo.h"

class Core : public QAbstractItemModel
{
	Q_OBJECT

public:
	typedef ulong TorrentID_t;
private:
	libtorrent::session *Session_;
	typedef QList<QPair<TorrentID_t, libtorrent::torrent_handle> > HandleDict_t;
	HandleDict_t Handles_;
	QList<QString> Headers_;
	TorrentID_t CurrentID_;
	enum Columns
	{
		ColumnName = 0
		, ColumnDownloaded
		, ColumnUploaded
		, ColumnSize
		, ColumnProgress
		, ColumnState
		, ColumnSP
		, ColumnDSpeed
		, ColumnUSpeed 
		, ColumnRemaining 
	};
	int InterfaceUpdateTimer_;
public:
	static Core* Instance ();
	Core (QObject *parent = 0);
	void DoDelayedInit ();
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
	TorrentInfo GetTorrentStats (int);
	void AddFile (const QString&, const QString&);
	void RemoveTorrent (int);
	void PauseTorrent (int);
	void ResumeTorrent (int);
private:
	HandleDict_t::iterator FindTorrentByID (TorrentID_t); 
	HandleDict_t::const_iterator FindTorrentByID (TorrentID_t) const; 
	QString GetStringForState (libtorrent::torrent_status::state_t) const;
	bool CheckValidity (int);
	void ReadSettings ();
	void RestoreTorrents ();
private slots:
	void writeSettings ();
protected:
	virtual void timerEvent (QTimerEvent*);
signals:
	void error (QString);
	void torrentAdded (TorrentID_t);
	void torrentRemoved (TorrentID_t);
};

#endif

