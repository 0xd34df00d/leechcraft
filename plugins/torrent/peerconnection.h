#ifndef PEERCONNECTION_H
#define PEERCONNECTION_H
#include <QObject>
#include <QBitArray>
#include <QByteArray>
#include <QTcpSocket>
#include "block.h"

class TorrentPeer;

static const int MySpeedArrayRotateSize = 8;

/*! @brief Represents a connection with a remote peer.
 *
 *
 */
class PeerConnection : public QTcpSocket
{
	Q_OBJECT
public:
	/*! @brief Represents connection state flags.
	 */
	enum PeerConnectionStateFlag
	{
		/// We are choking the remote peer
		Choking = 0x1
		/// We are interested in the remote per
		, AmInterested = 0x2
		/// We are choked by the remote peer
		, Choked = 0x4
		/// Remote peer is interested in us
		, PeerInterested = 0x8
	};

	Q_DECLARE_FLAGS (PeerConnectionState, PeerConnectionStateFlag);
private:
	struct BlockInfo
	{
		quint32 Index_, Offset_, Length_;
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

	QTcpSocket *Socket_;
	PeerConnectionState State_;
	QByteArray IncomingBuffer_, OutgoingBuffer_;
	QList<BlockInfo> PendingBlocks_;
	quint32 PendingBlockSizes_;
	QList<Block> Incoming_;
	bool GotHandshake_;
	bool GotPeerID_;
	bool SentHandshake_;
	int NextLength_;
	quint64 UploadSpeeds_ [MySpeedArrayRotateSize];
	quint64 DownloadSpeeds_ [MySpeedArrayRotateSize];
	int SpeedTimer_;
	int TimeoutTimer_;
	int PendingRequestTimer_;
	int KATimer_;
	bool InvalidateTimeout_;
	QByteArray InfoHash_;
	QByteArray PeerID_;
	QBitArray PeerPieces_;
	TorrentPeer *TorrentPeer_;
public:
	PeerConnection (const QByteArray&, QObject *parent = 0);
	~PeerConnection ();

	void Initialize (const QByteArray&, quint32);
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
	void SendPieceNotification (quint32);
	void SendPieceList (const QBitArray&);
	void SendRequest (quint32, quint32, quint32);
	void SendCancel (quint32, quint32, quint32);
	void SendBlock (quint32, quint32, const QByteArray&);

	qint64 Read (qint64);
	qint64 Write (qint64);
	quint64 GetDownloadSpeed () const;
	quint64 GetUploadSpeed () const;

	bool CanTransfer () const;
	quint64 GetBytesAvailable () const;
	quint64 GetSocketBytes () const;
	quint64 GetBytesToWrite () const;

	void SetReadBufferSize (quint32);
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

