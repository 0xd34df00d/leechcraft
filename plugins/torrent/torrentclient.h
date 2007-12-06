#ifndef TORRENTCLIENT_H
#define TORRENTCLIENT_H
#include <QBitArray>
#include <QHostAddress>
#include "torrentpeer.h"

class MetaInfo;
class PeerConnection;
class TorrentPiece;
struct TorrentClientImp;
template<typename T> class QList;

class TorrentClient : public QObject
{
	Q_OBJECT
public:
	enum State
	{
		StateIdle
		, StatePaused
		, StateStopping
		, StatePreparing
		, StateSearching
		, StateConnecting
		, StateWarmingUp
		, StateDownloading
		, StateEndGame
		, StateSeeding
	};

	enum Error
	{
		ErrorUnknown
		, ErrorTorrentParse
		, ErrorInvalidTracker
		, ErrorFile
		, ErrorServer
	};
private:
	TorrentClientImp *TCI_;
	friend struct TorrentClientImp;
public:
	TorrentClient (QObject *parent = 0);
	virtual ~TorrentClient ();

	bool SetTorrent (const QString&);
	bool SetTorrent (const QByteArray&);

	const MetaInfo& GetMetaInfo () const;

	void SetMaxConnections (int);
	int GetMaxConnections () const;
	void SetDestinationFolder (const QString&);
	const QString& GetDestinationFolder () const;
	void SetDumpedState (const QByteArray&);
	QByteArray GetDumpedState () const;

	qint64 GetProgress () const;
	void SetDownloadedBytes (qint64);
	qint64 GetDownloadedBytes () const;
	void SetUploadedBytes (qint64);
	qint64 GetUploadedBytes () const;
	int GetConnectedPeerCount () const;
	int GetSeedCount () const;

	const QByteArray& GetPeerID () const;
	const QByteArray& GetInfoHash () const;
	quint16 GetServerPort () const;

	State GetState () const;
	const QString& GetStateString () const;
	Error GetError () const;
	const QString& GetErrorString () const;
signals:
	void stateChanged (TorrentClient::State);
	void error (TorrentClient::Error);

	void finished ();
	void peerInfoUpdated ();

	void dataSent (int);
	void dataReceived (int);
	void progressUpdated (int);
	void downloadRateUpdated (int);
	void uploadRateUpdated (int);
	
	void stopped ();
public slots:
	void start ();
	void stop ();
	void setPaused (bool);
	void setupIncomingConnection (PeerConnection*);
protected slots:
	virtual void timerEvent (QTimerEvent*);
private slots:
	void sendToPeer (int, int, int, const QByteArray&);
	void fullVerificationDone ();
	void pieceVerified (int, bool);
	void handleFileError ();
	void connectToPeers ();
	QList<TorrentPeer*> getWeighedFreePeers () const;
	void setupOutgoingConnection ();
	void initializeConnection (PeerConnection*);
	void removeClient ();
	void peerPiecesAvailable (const QBitArray&);
	void peerRequestsBlock (int, int, int);
	void blockReceived (int, int, const QByteArray&);
	void peerConnectionBytesWritten (qint64);
	void peerConnectionBytesReceived (qint64);
	int getBlocksLeftForPiece (const TorrentPiece*) const;

	void scheduleUploads ();
	void scheduleDownloads ();
	void schedulePieceForClient (PeerConnection*);
	void requestMore (PeerConnection*);
	int requestBlocks (PeerConnection*, TorrentPiece*, int);
	void peerChoked ();
	void peerUnchoked ();

	void addToPeerList (const QList<TorrentPeer>&);
	void trackerStopped ();

	void updateProgress (int progress = -1);
};

#endif

