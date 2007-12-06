#include "torrentserver.h"
#include "peerconnection.h"
#include "torrentclient.h"
#include "connectionmanager.h"
#include "ratecontroller.h"

Q_GLOBAL_STATIC (TorrentServer, TorrentServerInstance);

TorrentServer::TorrentServer ()
{
}

TorrentServer* TorrentServer::Instance ()
{
	return TorrentServerInstance ();
}

void TorrentServer::AddClient (TorrentClient *tc)
{
	Clients_ << tc;
}

void TorrentServer::RemoveClient (TorrentClient *tc)
{
	Clients_.removeAll (tc);
}

void TorrentServer::incomingConnection (int sd)
{
	PeerConnection *client = new PeerConnection (ConnectionManager::Instance ()->GetClientID (), this);

	if (Clients_.isEmpty ())
		client->abort ();

	if (ConnectionManager::Instance ()->CanAddConnection ())
	{
		if (client->setSocketDescriptor (sd))
		{
			connect (client, SIGNAL (infoHashReceived (const QByteArray&)), this, SLOT (processInfoHash (const QByteArray&)));
			connect (client, SIGNAL (error (QAbstractSocket::SocketError)), this, SLOT (removeClient ()));
			RateController::Instance ()->AddSocket (client);
			ConnectionManager::Instance ()->AddConnection (client);
			if (Clients_.size () == 1)
			{
				client->disconnect (client, 0, this, 0);
				Clients_.first ()->setupIncomingConnection (client);
			}
			return;
		}
	}

	delete client;
}

void TorrentServer::removeClient ()
{
	PeerConnection *peer = qobject_cast<PeerConnection*> (sender ());
	peer->deleteLater ();
	RateController::Instance ()->RemoveSocket (peer);
	ConnectionManager::Instance ()->RemoveConnection (peer);
}

void TorrentServer::processInfoHash (const QByteArray& hash)
{
	PeerConnection *peer = qobject_cast<PeerConnection*> (sender ());
	foreach (TorrentClient *client, Clients_)
	{
		if (client->GetState () >= TorrentClient::StateSearching && client->GetInfoHash () == hash)
		{
			peer->disconnect (peer, 0, this, 0);
			client->setupIncomingConnection (peer);
			return;
		}
	}

	removeClient ();
}

