#ifndef TORRENTSERVER_H
#define TORRENTSERVER_H
#include <QList>
#include <QTcpServer>

class TorrentClient;

class TorrentServer : public QTcpServer
{
	Q_OBJECT

	QList<TorrentClient*> Clients_;
public:
	TorrentServer ();
	static TorrentServer* Instance ();

	void AddClient (TorrentClient*);
	void RemoveClient (TorrentClient*);
protected:
	void incomingConnection (int);
private slots:
	void removeClient ();
	void processInfoHash (const QByteArray&);
};

#endif

