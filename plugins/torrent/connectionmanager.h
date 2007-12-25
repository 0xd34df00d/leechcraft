#ifndef CONNECTIONMANAGER_H
#define CONNECTIONMANAGER_H
#include <QByteArray>
#include <QSet>

class PeerConnection;

class ConnectionManager
{
	QSet<PeerConnection*> Connections_;
	mutable QByteArray ID_;
public:
	static ConnectionManager *Instance ();
	bool CanAddConnection () const;
	void AddConnection (PeerConnection*);
	void RemoveConnection (PeerConnection*);
	int GetMaxConnections () const;
	QByteArray GetClientID () const;
};

#endif

