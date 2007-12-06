#ifndef TRACKERCLIENT_H
#define TRACKERCLIENT_H
#include <QByteArray>
#include <QList>
#include <QObject>
#include <QHostAddress>
#include <QHttp>
#include "metainfo.h"
#include "torrentpeer.h"

class TorrentClient;

class TrackerClient : public QObject
{
	Q_OBJECT

	TorrentClient *TorrentClient_;
	int RequestInterval_, RequestIntervalTimer_;
	QHttp Http_;
	MetaInfo MetaInfo_;
	QByteArray TrackerID_;
	QList<TorrentPeer> Peers_;
	qint64 Uploaded_, Downloaded_, Length_;
	bool FirstTrackerRequest_, LastTrackerRequest_;
public:
	TrackerClient (TorrentClient*, QObject *parent = 0);

	void Start (const MetaInfo&);
	void Stop ();

	qint64 GetUploadCount () const;
	qint64 GetDownloadCount () const;
	void SetUploadCount (qint64);
	void SetDownloadCount (qint64);
signals:
	void connectionError (QHttp::Error);
	void failure (const QString&);
	void warning (const QString&);
	void peerListUpdated (const QList<TorrentPeer>&);
	void uploadCountUpdated (qint64);
	void downloadCountUpdated (qint64);
	void stopped ();
protected:
	virtual void timerEvent (QTimerEvent*);
private slots:
	void fetchPeerList ();
	void httpRequestDone (bool);
};

#endif

