#ifndef PEERCONNECTION_H
#define PEERCONNECTION_H
#include <QBitArray>
#include <QList>
#include <QTcpSocket>
#include "block.h"

class TorrentPeer;

static const int MySpeedArrayRotateSize = 8;

class PeerConnection : public QTcpSocket
{
	Q_OBJECT
public:
	enum PeerConnectionStateFlag
	{
		Choking = 0x1
		, AmInterested = 0x2
		, Choked = 0x4
		, PeerInterested = 0x8
	};

	Q_DECLARE_FLAGS (PeerConnectionState, PeerConnectionStateFlag);
private:
	struct BlockInfo
	{
		int Index_, Offset_, Length_;
		QByteArray Block_;
	};

	enum Type
	{
		TypeChoke = 0
		, TypeUnchoke = 1
		, TypeInterested = 2
		, TypeNotInterested = 3
		, TypeHave = 4
		, TypeBitfield = 5
		, TypeRequest = 6
		, TypePiece = 7
		, TypeCancel = 8
		, TypePort = 9
	};

	QTcpSocket Socket_;
	PeerConnectionState State_;
	QByteArray IncomingBuffer_, OutgoingBuffer_, InfoHash_, PeerID_;
	QList<BlockInfo> PendingBlocks_;
	int PendingBlockSizes_, NextLength_, SpeedTimer_, TimeoutTimer_, PendingRequestTimer_, KATimer_;
	QList<Block> Incoming_;
	bool GotHandshake_, GotPeerID_, SentHandshake_, InvalidateTimeout_;
	qint64 UploadSpeeds_ [MySpeedArrayRotateSize], DownloadSpeeds_ [MySpeedArrayRotateSize];
	QBitArray PeerPieces_;
	TorrentPeer *TorrentPeer_;
public:
	PeerConnection (const QByteArray&, QObject *parent = 0);
	~PeerConnection ();

	void Initialize (const QByteArray&, int);
	void SetPeer (TorrentPeer*);
	TorrentPeer* GetPeer () const;

	PeerConnectionState GetPeerConnectionState () const;
	QBitArray GetAvailablePieces () const;
	QList<Block> GetInBlocks () const;

	void SendChoke ();
	void SendUnchoke ();
	void SendKA ();
	void SendInterested ();
	void SendNotInterested ();
	void SendPieceNotification (int);
	void SendPieceList (const QBitArray&);
	void SendRequest (int, int, int);
	void SendCancel (int, int, int);
	void SendBlock (int, int, const QByteArray&);

	qint64 Read (qint64);
	qint64 Write (qint64);
	qint64 GetDownloadSpeed () const;
	qint64 GetUploadSpeed () const;

	bool CanTransfer () const;
	qint64 GetBytesAvailable () const;
	qint64 GetSocketBytes () const;
	qint64 GetBytesToWrite () const;

	void SetReadBufferSize (int);
protected:
	virtual void timerEvent (QTimerEvent*);
	virtual qint64 readData (char*, qint64);
	virtual qint64 readLineData (char*, qint64);
	virtual qint64 writeData (const char*, qint64);
protected slots:
	virtual void connectToHostImplementation (const QString&, quint16, OpenMode om = ReadWrite);
	virtual void disconnectFromHostImplementation ();
private slots:
	void handleStateChange (QAbstractSocket::SocketState);
	void sendHandshake ();
	void processIncomingData ();
private:
	QString DecryptPeerID (const QByteArray&);
signals:
	void readyToTransfer ();
	void bytesReceived (qint64);
	void infoHashReceived (const QByteArray&);
	void choked ();
	void unchoked ();
	void interested ();
	void notInterested ();
	void piecesAvailable (const QBitArray&);
	void blockRequested (int, int, int);
	void blockReceived (int, int, const QByteArray&);
};

#endif

