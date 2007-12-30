#ifndef CORE_H
#define CORE_H
#include <QAbstractItemModel>
#include <QPair>
#include <QList>
#include <torrent_info.hpp>
#include <torrent_handle.hpp>
#include <session.hpp>
#include "torrentinfo.h"
#include "overallstats.h"
#include "torrentstruct.h"

class Core : public QAbstractItemModel
{
	Q_OBJECT

private:
	struct TorrentStruct
	{
		quint64 UploadedBefore_;
		libtorrent::torrent_handle Handle_;
	};

	libtorrent::session *Session_;
	typedef QList<TorrentStruct> HandleDict_t;
	HandleDict_t Handles_;
	QList<QString> Headers_;
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
	TorrentInfo GetTorrentStats (int) const;
	OverallStats GetOverallStats () const;
	void AddFile (const QString&, const QString&);
	void RemoveTorrent (int);
	void PauseTorrent (int);
	void ResumeTorrent (int);
private:
	QString GetStringForState (libtorrent::torrent_status::state_t) const;
	bool CheckValidity (int) const;
	void ReadSettings ();
	void RestoreTorrents ();
private slots:
	void writeSettings ();
protected:
	virtual void timerEvent (QTimerEvent*);
signals:
	void error (QString);
};

#endif

