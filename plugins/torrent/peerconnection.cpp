#include <QTimerEvent>
#include <plugininterface/tcpsocket.h>
#include "peerconnection.h"
#include "settingsmanager.h"
#include "torrentpeer.h"

namespace
{
	const int MinimalHeaderSize = 48;
	const int FullHeaderSize = 68;
	const char ProtocolID [] = "BitTorrent protocol";
	const char ProtocolIDSize = 19;

	quint32 FromNetworkData (const char *data)
	{
		const unsigned char *udata = reinterpret_cast<const unsigned char*> (data);
		return (static_cast<quint32> (udata [0]) << 24) |
			   (static_cast<quint32> (udata [1]) << 16) | 
			   (static_cast<quint32> (udata [2]) << 8) |
			   (static_cast<quint32> (udata [3]));
	}

	void ToNetworkData (quint32 num, char *data)
	{
		unsigned char *udata = reinterpret_cast<unsigned char*> (data);
		udata [3] = (num & 0xff);
		udata [2] = (num & 0xff00) >> 8;
		udata [1] = (num & 0xff0000) >> 16;
		udata [0] = (num & 0xff000000) >> 24;
	}
};

PeerConnection::PeerConnection (const QByteArray& id, QObject *parent)
: QTcpSocket (parent)
, State_ (Choking | Choked)
, PendingBlockSizes_ (0)
, GotHandshake_ (false)
, SentHandshake_ (false)
, GotPeerID_ (false)
, NextLength_ (-1)
, PendingRequestTimer_ (0)
, KATimer_ (0)
, InvalidateTimeout_ (false)
, PeerID_ (id)
, TorrentPeer_ (0)
{
	Socket_ = new QTcpSocket (parent);
	std::memset (UploadSpeeds_, 0, sizeof (UploadSpeeds_));
	std::memset (DownloadSpeeds_, 0, sizeof (DownloadSpeeds_));

	SpeedTimer_ = startTimer (SettingsManager::Instance ()->GetRateControlTimerDelay ());
	TimeoutTimer_ = startTimer (SettingsManager::Instance ()->GetConnectTimeout ());

	connect (this, SIGNAL (readyRead ()), this, SIGNAL (readyToTransfer ()));
	connect (this, SIGNAL (connected ()), this, SIGNAL (readyToTransfer ()));
	connect (Socket_, SIGNAL (connected ()), this, SIGNAL (connected ()));
	connect (Socket_, SIGNAL (readyRead ()), this, SIGNAL (readyRead ()));
	connect (Socket_, SIGNAL (disconnected ()), this, SIGNAL (disconnected ()));
	connect (Socket_, SIGNAL (error (QAbstractSocket::SocketError)), this, SIGNAL (error (QAbstractSocket::SocketError)));
	connect (Socket_, SIGNAL (bytesWritten (qint64)), this, SIGNAL (bytesWritten (qint64)));
	connect (Socket_, SIGNAL (stateChanged (QAbstractSocket::SocketState)), this, SLOT (handleStateChange (QAbstractSocket::SocketState)));
}

PeerConnection::~PeerConnection ()
{
}

void PeerConnection::Initialize (const QByteArray& infoHash, quint32 pieceCount)
{
	InfoHash_ = infoHash;
	PeerPieces_.resize (pieceCount);
	if (!SentHandshake_)
		sendHandshake ();
}

void PeerConnection::SetPeer (TorrentPeer *tp)
{
	TorrentPeer_ = tp;
}

TorrentPeer* PeerConnection::GetPeer () const
{
	return TorrentPeer_;
}

PeerConnection::PeerConnectionState PeerConnection::GetPeerConnectionState () const
{
	return State_;
}

QBitArray PeerConnection::GetAvailablePieces () const
{
	return PeerPieces_;
}

QList<Block> PeerConnection::GetInBlocks () const
{
	return Incoming_;
}

void PeerConnection::SendChoke ()
{
	const char msg [] = { 0, 0, 0, 1, 0 };
	write (msg, sizeof (msg));
	State_ |= Choking;

	PendingBlocks_.clear ();
	PendingBlockSizes_ = 0;
}

void PeerConnection::SendUnchoke ()
{
	const char msg [] = { 0, 0, 0, 1, 1 };
	write (msg, sizeof (msg));
	State_ &= ~Choking;

	if (PendingRequestTimer_)
		killTimer (PendingRequestTimer_);
}

void PeerConnection::SendKA ()
{
	const char msg [] = { 0, 0, 0, 0 };
	write (msg, sizeof (msg));
}

void PeerConnection::SendInterested ()
{
	const char msg [] = { 0, 0, 0, 1, 2 };
	write (msg, sizeof (msg));
	State_ |= AmInterested;

	if (PendingRequestTimer_)
		killTimer (PendingRequestTimer_);
	PendingRequestTimer_ = startTimer (SettingsManager::Instance ()->GetPendingRequestTimeout ());
}

void PeerConnection::SendNotInterested ()
{
	const char msg [] = { 0, 0, 0, 1, 3 };
	write (msg, sizeof (msg));

	State_ &= ~AmInterested;
}

void PeerConnection::SendPieceNotification (quint32 piece)
{
	if (!SentHandshake_)
		sendHandshake ();

	char msg [] = { 0, 0, 0, 5, 4, 0, 0, 0, 0 };
	ToNetworkData (piece, &(msg [5]));
	write (msg, sizeof (msg));
}

void PeerConnection::SendPieceList (const QBitArray& bf)
{
	if (!SentHandshake_)
		sendHandshake ();

	if (bf.count (true) == 0)
		return;

	int bfsize = bf.size ();
	int size = (bfsize + 7) / 8;
	QByteArray bits (size, '\0');
	for (int i = 0; i < bfsize; ++i)
		if (bf.testBit (i))
		{
			quint32 byte = static_cast<quint32> (i) / 8;
			quint32 bit = static_cast<quint32> (i) % 8;
			bits [byte] = static_cast<uchar> (bits.at (byte)) | (1 << (7 - bit));
		}

	char msg [] = { 0, 0, 0, 1, 5 };
	ToNetworkData (bits.size () + 1, &(msg [0]));
	write (msg, sizeof (msg));
	write (bits);
}

void PeerConnection::SendRequest (quint32 index, quint32 offset, quint32 length)
{
	char msg [] = { 0, 0, 0, 1, 6 };
	ToNetworkData (13, &(msg [0]));
	write (msg, sizeof (msg));

	char numbers [4 * 3] = { 0 };
	ToNetworkData (index, &(numbers [0]));
	ToNetworkData (offset, &(numbers [4]));
	ToNetworkData (length, &(numbers [8]));
	write (numbers, sizeof (numbers));

	Incoming_ << Block (index, offset, length);

	if (PendingRequestTimer_)
		killTimer (PendingRequestTimer_);
	PendingRequestTimer_ = startTimer (SettingsManager::Instance ()->GetPendingRequestTimeout ());
}

void PeerConnection::SendCancel (quint32 index, quint32 offset, quint32 length)
{
	char msg [] = { 0, 0, 0, 1, 8 };
	ToNetworkData (13, &(msg [0]));
	write (msg, sizeof (msg));
	
	char numbers [4 * 3] = { 0 };
	ToNetworkData (index, &(numbers [0]));
	ToNetworkData (offset, &(numbers [4]));
	ToNetworkData (length, &(numbers [8]));
	write (numbers, sizeof (numbers));

	Incoming_.removeAll (Block (index, offset, length));
}

void PeerConnection::SendBlock (quint32 index, quint32 offset, const QByteArray& data)
{
	QByteArray block;

	char msg [] = { 0, 0, 0, 1, 7 };

	ToNetworkData (9 + data.size (), &(msg [0]));
	block += QByteArray (msg, sizeof (msg));

	char numbers [4 * 2];
	ToNetworkData (index, &(numbers [0]));
	ToNetworkData (offset, &(numbers [4]));
	block += QByteArray (numbers, sizeof (numbers));

	block += data;

	BlockInfo bi;
	bi.Index_ = index;
	bi.Offset_ = offset;
	bi.Length_ = block.size ();
	bi.Block_ = block;

	PendingBlocks_ << bi;
	PendingBlockSizes_ += block.size ();

	if (PendingBlockSizes_ > 32 * 16384)
	{
		SendChoke ();
		SendUnchoke ();
		return;
	}

	emit readyToTransfer ();
}

qint64 PeerConnection::Read (qint64 bytes)
{
	char buffer [1024];
	qint64 totalRead = 0;
	do
	{
		qint64 bytesRead = Socket_->read (buffer, qMin<qint64> (bytes - totalRead, sizeof (buffer)));
		if (bytesRead < 0)
			break;

		qint64 oldSize = IncomingBuffer_.size ();
		IncomingBuffer_.resize (oldSize + bytesRead);
		std::memcpy (IncomingBuffer_.data () + oldSize, buffer, bytesRead);

		totalRead += bytesRead;
	} while (totalRead < bytes);

	if (totalRead > 0)
	{
		DownloadSpeeds_ [0] += totalRead;
		emit bytesReceived (totalRead);
		processIncomingData ();
	}

	return totalRead;
}

qint64 PeerConnection::Write (qint64 bytes)
{
	qint64 totalWritten = 0;
	do
	{
		if (OutgoingBuffer_.isEmpty () && !PendingBlocks_.isEmpty ())
		{
			BlockInfo bi = PendingBlocks_.takeFirst ();
			PendingBlockSizes_ -= bi.Length_;
			OutgoingBuffer_ += bi.Block_;
		}
		qint64 written = Socket_->write (OutgoingBuffer_.constData (), qMin<qint64> (bytes - totalWritten, OutgoingBuffer_.size ()));

		if (written <= 0)
			return totalWritten ? totalWritten : written;
		
		totalWritten += written;
		UploadSpeeds_ [0] += written;
		OutgoingBuffer_.remove (0, written);
	} while (totalWritten < bytes && (!OutgoingBuffer_.isEmpty () || !PendingBlocks_.isEmpty ()));

	return totalWritten;
}

quint64 PeerConnection::GetDownloadSpeed () const
{
	quint64 sum = 0;
	for (uint i = 0; i < sizeof (DownloadSpeeds_) / sizeof (DownloadSpeeds_ [0]); ++i)
		sum += DownloadSpeeds_ [i];
	return sum / (MySpeedArrayRotateSize * 2);
}

quint64 PeerConnection::GetUploadSpeed () const
{
	quint64 sum = 0;
	for (uint i = 0; i < sizeof (UploadSpeeds_) / sizeof (UploadSpeeds_ [0]); ++i)
		sum += UploadSpeeds_ [i];
	return sum / (MySpeedArrayRotateSize * 2);
}

bool PeerConnection::CanTransfer () const
{
	return GetBytesAvailable () > 0 || Socket_->bytesAvailable () > 0 || !OutgoingBuffer_.isEmpty () || !PendingBlocks_.isEmpty ();
}

quint64 PeerConnection::GetBytesAvailable () const
{
	return IncomingBuffer_.size  () + Socket_->bytesAvailable ();
}

quint64 PeerConnection::GetSocketBytes () const
{
	return Socket_->bytesAvailable ();
}

quint64 PeerConnection::GetBytesToWrite () const
{
	return Socket_->bytesToWrite ();
}

void PeerConnection::SetReadBufferSize (quint32 size)
{
	Socket_->setReadBufferSize (size);
}

void PeerConnection::connectToHostImplementation (const QString& host, quint16 port, OpenMode om)
{
	setOpenMode (om);
	Socket_->connectToHost (host, port, om);
}

void PeerConnection::disconnectFromHostImplementation ()
{
	Socket_->disconnectFromHost ();
}

void PeerConnection::timerEvent (QTimerEvent *e)
{
	if (e->timerId () == SpeedTimer_)
	{
		for (int i = MySpeedArrayRotateSize - 2; i >= 0; --i)
		{
			UploadSpeeds_ [i + 1] = UploadSpeeds_ [i];
			DownloadSpeeds_ [i + 1] = DownloadSpeeds_ [i];
		}
		UploadSpeeds_ [0] = 0;
		DownloadSpeeds_ [0] = 0;
	}
	else if (e->timerId () == TimeoutTimer_)
	{
		if (InvalidateTimeout_)
			InvalidateTimeout_ = false;
		else
			abort ();
	}
	else if (e->timerId () == PendingRequestTimer_)
		abort ();
	else if (e->timerId () == KATimer_)
		SendKA ();
	
	QTcpSocket::timerEvent (e);
}

qint64 PeerConnection::readData (char *data, qint64 length)
{
	qint64 n = qMin<qint64> (length, IncomingBuffer_.size ());
	std::memcpy (data, IncomingBuffer_.constData (), n);
	IncomingBuffer_.remove (0, n);
	return n;
}

qint64 PeerConnection::readLineData (char *data, qint64 length)
{
	return QIODevice::readLineData (data, length);
}

qint64 PeerConnection::writeData (const char *data, qint64 length)
{
	int oldSize = OutgoingBuffer_.size ();
	OutgoingBuffer_.resize (oldSize + length);
	memcpy (OutgoingBuffer_.data () + oldSize, data, length);
	emit readyToTransfer ();
	return length;
}

void PeerConnection::handleStateChange (QAbstractSocket::SocketState st)
{
	setLocalAddress (Socket_->localAddress ());
	setLocalPort (Socket_->localPort ());
	setPeerName (Socket_->peerName ());
	setPeerAddress (Socket_->peerAddress ());
	setPeerPort (Socket_->peerPort ());
	setSocketState (st);
}

void PeerConnection::sendHandshake ()
{
	SentHandshake_ = true;

	if (TimeoutTimer_)
		killTimer (TimeoutTimer_);
	TimeoutTimer_ = startTimer (SettingsManager::Instance ()->GetClientTimeout ());

	write (&ProtocolIDSize, 1);
	write (ProtocolID, ProtocolIDSize);
	write (QByteArray (8, '\0'));
	write (InfoHash_);
	write (PeerID_);
}

void PeerConnection::processIncomingData ()
{
	InvalidateTimeout_ = true;
	if (!GotHandshake_)
	{
		if (bytesAvailable () < MinimalHeaderSize)
			return;

		QByteArray id = read (ProtocolIDSize + 1);
		if (id.at (0) != ProtocolIDSize || !id.mid (1).startsWith (ProtocolID))
		{
			abort ();
			return;
		}

		read (8);

		QByteArray peerInfoHash = read (20);
		if (!peerInfoHash.isEmpty () && peerInfoHash != InfoHash_)
		{
			abort ();
			return;
		}

		emit infoHashReceived (peerInfoHash);
		if (InfoHash_.isEmpty ())
		{
			abort ();
			return;
		}

		if (!SentHandshake_)
			sendHandshake ();
		GotHandshake_ = true;
	}

	if (!GotPeerID_)
	{
		if (bytesAvailable () < 20)
			return;
		GotPeerID_ = true;
		QByteArray remotePeerID = read (20);
		if (remotePeerID == PeerID_)
		{
			abort ();
			return;
		}
		DecryptPeerID (remotePeerID);
	}

	if (!KATimer_)
		KATimer_ = startTimer (SettingsManager::Instance ()->GetKAInterval ());

	do
	{
		if (NextLength_ == -1)
		{
			if (bytesAvailable () < 4)
				return;

			char tmp [4];
			read (tmp, sizeof (tmp));
			NextLength_ = FromNetworkData (tmp);

			if (NextLength_ < 0 || NextLength_ > 200000)
			{
				abort ();
				return;
			}
		}

		if (NextLength_ == 0)
		{
			NextLength_ = -1;
			continue;
		}

		if (bytesAvailable () < NextLength_)
			return;

		QByteArray packet = read (NextLength_);
		if (packet.size () != NextLength_)
		{
			abort ();
			return;
		}

		switch (packet.at (0))
		{
			case TypeChoke:
				State_ |= Choked;
				Incoming_.clear ();
				if (PendingRequestTimer_)
					killTimer (PendingRequestTimer_);
				emit choked ();
				break;
			case TypeUnchoke:
				State_ &= ~ Choked;
				emit unchoked ();
				break;
			case TypeInterested:
				State_ |= PeerInterested;
				emit interested ();
				break;
			case TypeNotInterested:
				State_ &= ~PeerInterested;
				emit notInterested ();
				break;
			case TypeHave:
				{
					quint32 index = FromNetworkData (&packet.data () [1]);
					if (index < static_cast<quint32> (PeerPieces_.size ()))
						PeerPieces_.setBit (static_cast<int> (index));
				}
				emit piecesAvailable (GetAvailablePieces ());
				break;
			case TypeBitfield:
				for (int i = 1; i < packet.size (); ++i)
					for (int bit = 0; bit < 8; ++bit)
						if (packet.at (i) & (1 << (7 - bit)))
						{
							int bitIndex = static_cast<int> (((i - 1) * 8 + bit));
							if (bitIndex >= 0 && bitIndex < PeerPieces_.size ())
								PeerPieces_.setBit (bitIndex);
						}
				emit piecesAvailable (GetAvailablePieces ());
				break;
			case TypeRequest:
				{
					quint32 index = FromNetworkData (&packet.data () [1]);
					quint32 begin = FromNetworkData (&packet.data () [5]);
					quint32 length = FromNetworkData (&packet.data () [9]);
					emit blockRequested (static_cast<int> (index), static_cast<int> (begin), static_cast<int> (length));
				}
				break;
			case TypePiece:
				{
					int index = static_cast<int> (FromNetworkData (&packet.data () [1]));
					int begin = static_cast<int> (FromNetworkData (&packet.data () [5]));
					Incoming_.removeAll (Block (index, begin, packet.size () - 9));

					emit blockReceived (index, begin, packet.mid (9));

					if (PendingRequestTimer_)
					{
						killTimer (PendingRequestTimer_);
						PendingRequestTimer_ = 0;
					}
				}
				break;
			case TypeCancel:
				{
					quint32 index = FromNetworkData (&packet.data () [1]);
					quint32 begin = FromNetworkData (&packet.data () [5]);
					quint32 length = FromNetworkData (&packet.data () [9]);
					for (int i = 0; i < PendingBlocks_.size (); ++i)
					{
						const BlockInfo& bi = PendingBlocks_ [i];
						if (bi.Index_ == index &&
							bi.Offset_ == begin &&
							bi.Length_ == length)
						{
							PendingBlocks_.removeAt (i);
							break;
						}
					}
				}
				break;
			default:
				break;
		}
		NextLength_ = -1;
	} while (bytesAvailable () > 0);
}

QString PeerConnection::DecryptPeerID (const QByteArray& peerid)
{
	Q_UNUSED (peerid);
	return QString ();
}

