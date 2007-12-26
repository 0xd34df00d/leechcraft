#include <QtCore>
#include <QtDebug>
#include "torrentclient.h"
#include "trackerclient.h"
#include "filemanager.h"
#include "connectionmanager.h"
#include "torrentserver.h"
#include "ratecontroller.h"
#include "settingsmanager.h"
#include "peerconnection.h"

extern "C"
{
#include "sha1/sha1.h"
}

struct TorrentPiece
{
	int Index_;
	int Length_;
	QBitArray Completed_;
	QBitArray Requested_;
	bool InProgress_;
};

struct TorrentClientImp
{
	TorrentClientImp (TorrentClient*);
	void CallScheduler ();
	void CallPeerConnector ();

	void SetError (TorrentClient::Error);
	void SetState (TorrentClient::State);
	TorrentClient::Error Error_;
	TorrentClient::State State_;
	QString ErrorString_, StateString_, ExtentedErrorString_, Destination_;
	MetaInfo MetaInfo_;

	QByteArray PeerID_, InfoHash_;
	TrackerClient TrackerClient_;
	FileManager FileManager_;

	QList<PeerConnection*> Connections_;
	QList<TorrentPeer*> Peers_;
	bool SchedulerCalled_, ConnectingToClients_;
	int UploadScheduleTimer_;

	QMap<int, PeerConnection*> ReadIDs_;
	QMultiMap<PeerConnection*, TorrentPiece*> Payloads_;
	QMap<int, TorrentPiece*> PendingPieces_;
	QBitArray CompletedPieces_, IncompletePieces_;
	int PieceCount_;

	int LastProgressValue_;
	qint64 DownloadedBytes_, UploadedBytes_;
	QVector<int> DownloadRate_, UploadRate_;
	int TransferRateTimer_;

	TorrentClient *Q_;
};

TorrentClientImp::TorrentClientImp (TorrentClient *tc)
: Error_ (TorrentClient::ErrorUnknown)
, State_ (TorrentClient::StateIdle)
, TrackerClient_ (tc)
, SchedulerCalled_ (false)
, ConnectingToClients_ (false)
, UploadScheduleTimer_ (0)
, PieceCount_ (0)
, LastProgressValue_ (-1)
, DownloadedBytes_ (0)
, UploadedBytes_ (0)
, DownloadRate_ (SettingsManager::Instance ()->GetRateControlWindowLength (), 0)
, UploadRate_ (SettingsManager::Instance ()->GetRateControlWindowLength (), 0)
, TransferRateTimer_ (0)
, Q_ (tc)
{
	ErrorString_ = QT_TRANSLATE_NOOP (TorrentClient, "UnknownError");
	StateString_ = QT_TRANSLATE_NOOP (TorrentClient, "Idle");
}

void TorrentClientImp::SetError (TorrentClient::Error error)
{
	Error_ = error;
	switch (error)
	{
		case TorrentClient::ErrorUnknown:
			ErrorString_ = QT_TRANSLATE_NOOP (TorrentClient, "Unknown error");
			break;
		case TorrentClient::ErrorTorrentParse:
			ErrorString_ = QT_TRANSLATE_NOOP (TorrentClient, "Invalid torrent data");
			break;
		case TorrentClient::ErrorInvalidTracker:
			ErrorString_ = QT_TRANSLATE_NOOP (TorrentClient, "Tracker communication error");
			break;
		case TorrentClient::ErrorFile:
			ErrorString_ = QT_TRANSLATE_NOOP (TorrentClient, "File error");
			break;
		case TorrentClient::ErrorServer:
			ErrorString_ = QT_TRANSLATE_NOOP (TorrentClient, "Unable to initialize server");
			break;
	}
	emit Q_->error (error);
}

void TorrentClientImp::SetState (TorrentClient::State state)
{
	State_ = state;
	switch (state)
	{
		case TorrentClient::StateIdle:
			StateString_ = QT_TRANSLATE_NOOP (TorrentClient, "Idle");
			break;
		case TorrentClient::StatePaused:
			StateString_ = QT_TRANSLATE_NOOP (TorrentClient, "Paused");
			break;
		case TorrentClient::StateStopping:
			StateString_ = QT_TRANSLATE_NOOP (TorrentClient, "Stopping");
			break;
		case TorrentClient::StatePreparing:
			StateString_ = QT_TRANSLATE_NOOP (TorrentClient, "Preparing");
			break;
		case TorrentClient::StateSearching:
			StateString_ = QT_TRANSLATE_NOOP (TorrentClient, "Searching");
			break;
		case TorrentClient::StateConnecting:
			StateString_ = QT_TRANSLATE_NOOP (TorrentClient, "Connecting");
			break;
		case TorrentClient::StateWarmingUp:
			StateString_ = QT_TRANSLATE_NOOP (TorrentClient, "Warming up");
			break;
		case TorrentClient::StateDownloading:
			StateString_ = QT_TRANSLATE_NOOP (TorrentClient, "Downloading");
			break;
		case TorrentClient::StateEndGame:
			StateString_ = QT_TRANSLATE_NOOP (TorrentClient, "Finishing");
			break;
		case TorrentClient::StateSeeding:
			StateString_ = QT_TRANSLATE_NOOP (TorrentClient, "Seeding");
			break;
	}
	emit Q_->stateChanged (state);
}

void TorrentClientImp::CallScheduler ()
{
	if (!SchedulerCalled_)
	{
		SchedulerCalled_ = true;
		QMetaObject::invokeMethod (Q_, "scheduleDownloads", Qt::QueuedConnection);
	}
}

void TorrentClientImp::CallPeerConnector ()
{
	if (!ConnectingToClients_)
	{
		ConnectingToClients_ = true;
		QTimer::singleShot (10000, Q_, SLOT (connectToPeers ()));
	}
}

TorrentClient::TorrentClient (QObject *parent)
: QObject (parent)
, TCI_ (new TorrentClientImp (this))
{
	connect (&TCI_->FileManager_, SIGNAL (dataRead (int, int, int, const QByteArray&)), this, SLOT (sendToPeer (int, int, int, const QByteArray&)));
	connect (&TCI_->FileManager_, SIGNAL (verificationProgress (int)), this, SLOT (updateProgress (int)));
	connect (&TCI_->FileManager_, SIGNAL (verificationDone ()), this, SLOT (fullVerificationDone ()));
	connect (&TCI_->FileManager_, SIGNAL (pieceVerified (int, bool)), this, SLOT (pieceVerified (int, bool)));
	connect (&TCI_->FileManager_, SIGNAL (error ()), this, SLOT (handleFileError ()));

	connect (&TCI_->TrackerClient_, SIGNAL (peerListUpdated (const QList<TorrentPeer>&)), this, SLOT (addToPeerList (const QList<TorrentPeer>&)));
	connect (&TCI_->TrackerClient_, SIGNAL (stopped ()), this, SIGNAL (stopped ()));
}

TorrentClient::~TorrentClient ()
{
	qDeleteAll (TCI_->Peers_);
	qDeleteAll (TCI_->PendingPieces_);
	delete TCI_;
}

bool TorrentClient::SetTorrent (const QString& name)
{
	QFile file (name);
	if (!file.open (QIODevice::ReadOnly) || !SetTorrent (file.readAll ()))
	{
		TCI_->SetError (ErrorTorrentParse);
		return false;
	}
	return true;
}

bool TorrentClient::SetTorrent (const QByteArray& data)
{
	if (!TCI_->MetaInfo_.Parse (data))
	{
		TCI_->SetError (ErrorTorrentParse);
		return false;
	}

	QByteArray infoValue = TCI_->MetaInfo_.GetInfo ();
	SHA1Context sha;
	SHA1Reset (&sha);
	SHA1Input (&sha, reinterpret_cast<const unsigned char*> (infoValue.constData ()), infoValue.size ());
	SHA1Result (&sha);
	unsigned char *digest = reinterpret_cast<unsigned char*> (sha.Message_Digest);
	TCI_->InfoHash_.resize (20);

	for (int i = 0; i < 5; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
			TCI_->InfoHash_ [i * 4 + j] = digest [i * 4 + j];
#else
			TCI_->InfoHash_ [i * 4 + j] = digest [i * 4 + 3 - j];
#endif
		}
	}
	return true;
}

const MetaInfo& TorrentClient::GetMetaInfo () const
{
	return TCI_->MetaInfo_;
}

void TorrentClient::SetDestinationFolder (const QString& dir)
{
	TCI_->Destination_ = dir;
}

const QString& TorrentClient::GetDestinationFolder () const
{
	return TCI_->Destination_;
}

void TorrentClient::SetDumpedState (const QByteArray& dumpedState)
{
	QDataStream stream (dumpedState);

	quint16 version = 0;
	stream >> version;
	if (version != 2)
		return;

	stream >> TCI_->CompletedPieces_;

	while (!stream.atEnd ())
	{
		int index, length;
		QBitArray completed;
		stream >> index >> length >> completed;
		if (stream.status () != QDataStream::Ok)
		{
			TCI_->CompletedPieces_.clear ();
			break;
		}

		TorrentPiece *piece = new TorrentPiece;
		piece->Index_ = index;
		piece->Length_ = length;
		piece->Completed_ = completed;
		piece->Requested_.resize (completed.size ());
		piece->InProgress_ = false;
		TCI_->PendingPieces_ [index] = piece;
	}
}

QByteArray TorrentClient::GetDumpedState () const
{
	QByteArray result;
	QDataStream stream (&result, QIODevice::WriteOnly);

	stream << (quint16) 2;
	stream << TCI_->CompletedPieces_;

	QMap<int, TorrentPiece*>::ConstIterator i = TCI_->PendingPieces_.constBegin ();
	while (i != TCI_->PendingPieces_.constEnd ())
	{
		TorrentPiece *piece = i.value ();
		if (getBlocksLeftForPiece (piece) > 0 && getBlocksLeftForPiece (piece) < piece->Completed_.size ())
		{
			stream << piece->Index_;
			stream << piece->Length_;
			stream << piece->Completed_;
		}
	}

	return result;
}

qint64 TorrentClient::GetProgress () const
{
	return TCI_->LastProgressValue_;
}

void TorrentClient::SetDownloadedBytes (qint64 bytes)
{
	TCI_->DownloadedBytes_ = bytes;
}

qint64 TorrentClient::GetDownloadedBytes () const
{
	return TCI_->DownloadedBytes_;
}

void TorrentClient::SetUploadedBytes (qint64 bytes)
{
	TCI_->UploadedBytes_ = bytes;
}

qint64 TorrentClient::GetUploadedBytes () const
{
	return TCI_->UploadedBytes_;
}

int TorrentClient::GetConnectedPeerCount () const
{
	int result = 0;
	foreach (PeerConnection *pc, TCI_->Connections_)
		if (pc->state () == QAbstractSocket::ConnectedState)
			++result;

	qDebug () << Q_FUNC_INFO << result;
	return result;
}

int TorrentClient::GetSeedCount () const
{
	int result = 0;
	foreach (PeerConnection *pc, TCI_->Connections_)
		if (pc->GetAvailablePieces ().count (true) == TCI_->PieceCount_)
			++result;

	qDebug () << Q_FUNC_INFO << result;
	return result;
}

const QByteArray& TorrentClient::GetPeerID () const
{
	return TCI_->PeerID_;
}

const QByteArray& TorrentClient::GetInfoHash () const
{
	return TCI_->InfoHash_;
}

TorrentClient::State TorrentClient::GetState () const
{
	return TCI_->State_;
}

const QString& TorrentClient::GetStateString () const
{
	return TCI_->StateString_;
}

TorrentClient::Error TorrentClient::GetError () const
{
	return TCI_->Error_;
}

const QString& TorrentClient::GetErrorString () const
{
	return TCI_->ErrorString_;
}

void TorrentClient::start ()
{
	if (TCI_->State_ != StateIdle)
		return;

	TorrentServer::Instance ()->AddClient (this);
	TCI_->SetState (StatePreparing);
	TCI_->FileManager_.SetMetaInfo (TCI_->MetaInfo_);
	TCI_->FileManager_.SetDestinationFolder (TCI_->Destination_);
	TCI_->FileManager_.SetCompletedPieces (TCI_->CompletedPieces_);
	TCI_->FileManager_.start (QThread::LowestPriority);
	TCI_->FileManager_.startDataVerification ();
}

void TorrentClient::stop ()
{
	if (TCI_->State_ == StateStopping)
		return;

	TorrentServer::Instance ()->RemoveClient (this);
	State oldState = TCI_->State_;
	TCI_->SetState (StateStopping);

	if (TCI_->TransferRateTimer_)
	{
		killTimer (TCI_->TransferRateTimer_);
		TCI_->TransferRateTimer_ = 0;
	}

	foreach (PeerConnection *pc, TCI_->Connections_)
	{
		RateController::Instance ()->RemoveSocket (pc);
		ConnectionManager::Instance ()->RemoveConnection (pc);
		pc->abort ();
	}
	TCI_->Connections_.clear ();

	if (oldState > StatePreparing)
		TCI_->TrackerClient_.Stop ();
	else
	{
		TCI_->SetState (StateIdle);
		emit stopped ();
	}
}

void TorrentClient::setPaused (bool paused)
{
	if (paused)
	{
		TCI_->SetState (StatePaused);
		foreach (PeerConnection *pc, TCI_->Connections_)
			pc->abort ();
		TCI_->Connections_.clear ();
		TorrentServer::Instance ()->RemoveClient (this);
	}
	else
	{
		TCI_->SetState (TCI_->CompletedPieces_.count (true) == TCI_->FileManager_.GetPieceCount () ? StateSeeding : StateSearching);
		connectToPeers ();
		TorrentServer::Instance ()->AddClient (this);
	}
}

void TorrentClient::setupIncomingConnection (PeerConnection *pc)
{
	initializeConnection (pc);

	RateController::Instance ()->AddSocket (pc);
	TCI_->Connections_ << pc;
	pc->Initialize (TCI_->InfoHash_, TCI_->PieceCount_);
	pc->SendPieceList (TCI_->CompletedPieces_);

	emit peerInfoUpdated ();

	if (TCI_->State_ == StateSearching || TCI_->State_ == StateConnecting)
	{
		if (!TCI_->CompletedPieces_.count (true))
			TCI_->SetState (StateWarmingUp);
		else if (TCI_->IncompletePieces_.count (true) < SettingsManager::Instance ()->GetEndGamePieces () && TCI_->PendingPieces_.size () > TCI_->IncompletePieces_.count (true))
			TCI_->SetState (StateEndGame);
	}

	if (TCI_->Connections_.isEmpty ())
		scheduleUploads ();
}

void TorrentClient::timerEvent (QTimerEvent *e)
{
	if (e->timerId () == TCI_->UploadScheduleTimer_)
	{
		scheduleUploads ();
		return;
	}
	if (e->timerId () != TCI_->TransferRateTimer_)
	{
		QObject::timerEvent (e);
		return;
	}

	qint64 uploadSpeed = 0, downloadSpeed = 0;
	int width = qMin<int> (TCI_->UploadRate_.size (), TCI_->DownloadRate_.size ());
	for (int i = 0; i < width; ++i)
	{
		uploadSpeed += TCI_->UploadRate_ [i];
		downloadSpeed += TCI_->DownloadRate_ [i];
	}
	uploadSpeed /= static_cast<qint64> (width);
	downloadSpeed /= static_cast<qint64> (width);
	for (int i = width - 2; i >= 0; --i)
	{
		TCI_->UploadRate_ [i + 1] = TCI_->UploadRate_ [i];
		TCI_->DownloadRate_ [i + 1] = TCI_->DownloadRate_ [i];
	}
	TCI_->UploadRate_ [0] = 0;
	TCI_->DownloadRate_ [0] = 0;

	emit uploadRateUpdated (uploadSpeed);
	emit downloadRateUpdated (downloadSpeed);

	if (!uploadSpeed && !downloadSpeed)
	{
		killTimer (TCI_->TransferRateTimer_);
		TCI_->TransferRateTimer_ = 0;
	}
}

void TorrentClient::sendToPeer (int readid, int index, int offset, const QByteArray& data)
{
	PeerConnection *pc = TCI_->ReadIDs_ [readid];
	if (pc)
		if  (!(pc->GetPeerConnectionState () & PeerConnection::Choking))
			pc->SendBlock (index, offset, data);
	TCI_->ReadIDs_.remove (readid);
}

void TorrentClient::fullVerificationDone ()
{
	TCI_->CompletedPieces_ = TCI_->FileManager_.GetCompletedPieces ();
	TCI_->IncompletePieces_.resize (TCI_->CompletedPieces_.size ());
	TCI_->PieceCount_ = TCI_->CompletedPieces_.size ();

	for (int i = 0; i < TCI_->FileManager_.GetPieceCount (); ++i)
		if (!TCI_->CompletedPieces_.testBit (i))
			TCI_->IncompletePieces_.setBit (i);

	updateProgress ();

	QMap<int, TorrentPiece*>::Iterator i = TCI_->PendingPieces_.begin ();
	while (i != TCI_->PendingPieces_.end ())
		if (TCI_->CompletedPieces_.testBit (i.key ()))
			i = TCI_->PendingPieces_.erase (i);
		else
			++i;

	TCI_->UploadScheduleTimer_ = startTimer (SettingsManager::Instance ()->GetUploadScheduleInterval ());

	TorrentServer *server = TorrentServer::Instance ();
	int minPort = SettingsManager::Instance ()->GetServerMinPort (),
		maxPort = SettingsManager::Instance ()->GetServerMaxPort ();
	if (!server->isListening ())
	{
		for (int i = minPort; i <= maxPort; ++i)
			if (server->listen (QHostAddress::Any, i))
				break;
		if (!server->isListening ())
		{
			TCI_->SetError (ErrorServer);
			TCI_->ExtentedErrorString_ = tr ("Could not bind to address.");
			return;
		}
	}

	TCI_->SetState (TCI_->CompletedPieces_.count (true) == TCI_->PieceCount_ ? StateSeeding : StateSearching);

	TCI_->TrackerClient_.SetUploadCount (TCI_->UploadedBytes_);
	TCI_->TrackerClient_.SetDownloadCount (TCI_->DownloadedBytes_);
	TCI_->TrackerClient_.Start (TCI_->MetaInfo_);
}

void TorrentClient::pieceVerified (int index, bool ok)
{
	qDebug () << Q_FUNC_INFO;
	TorrentPiece *piece = TCI_->PendingPieces_ [index];

	QMultiMap<PeerConnection*, TorrentPiece*>::Iterator i = TCI_->Payloads_.begin ();
	while (i != TCI_->Payloads_.end ())
		if (i.value ()->Index_ == index)
			i = TCI_->Payloads_.erase (i);
		else
			++i;

	if (!ok)
	{
		piece->InProgress_ = false;
		piece->Completed_.fill (false);
		piece->Requested_.fill (false);
		TCI_->CallScheduler ();
		return;
	}

	foreach (TorrentPeer *peer, TCI_->Peers_)
	{
		if (!peer->Interesting_)
			continue;
		bool interesting = false;
		for (int i = 0; i < TCI_->PieceCount_; ++i)
			if (peer->Pieces_.testBit (i) && TCI_->IncompletePieces_.testBit (i))
			{
				interesting = true;
				break;
			}
		peer->Interesting_ = interesting;
	}

	delete piece;
	TCI_->PendingPieces_.remove (index);
	TCI_->CompletedPieces_.setBit (index);
	TCI_->IncompletePieces_.clearBit (index);

	foreach (PeerConnection *pc, TCI_->Connections_)
		if (pc->state () == QAbstractSocket::ConnectedState && !pc->GetAvailablePieces ().testBit (index))
			pc->SendPieceNotification (index);

	int completed = TCI_->CompletedPieces_.count (true);
	if (completed == TCI_->PieceCount_)
	{
		if (TCI_->State_ != StateSeeding)
		{
			TCI_->SetState (StateSeeding);
			TCI_->TrackerClient_.Start (TCI_->MetaInfo_);
		}
	}
	else
	{
		if (completed == 1)
			TCI_->SetState (StateDownloading);
		else if (TCI_->IncompletePieces_.count (true) < SettingsManager::Instance ()->GetEndGamePieces () && TCI_->PendingPieces_.size () > TCI_->IncompletePieces_.count (true))
			TCI_->SetState (StateEndGame);
		TCI_->CallScheduler ();
	}

	updateProgress ();
}

void TorrentClient::handleFileError ()
{
	if (TCI_->State_ == StatePaused)
		return;
	setPaused (true);
	emit error (ErrorFile);
}

void TorrentClient::connectToPeers ()
{
	TCI_->ConnectingToClients_ = false;
	if (TCI_->State_ == StateStopping || TCI_->State_ == StateIdle || TCI_->State_ == StatePaused)
		return;

	if (TCI_->State_ == StateSearching)
		TCI_->SetState (StateConnecting);

	QList<TorrentPeer*> free = getWeighedFreePeers ();

	while (!free.isEmpty () && ConnectionManager::Instance ()->CanAddConnection () &&
			(qrand () % (ConnectionManager::Instance ()->GetMaxConnections () / 2)))
	{
		PeerConnection *pc = new PeerConnection (ConnectionManager::Instance ()->GetClientID (), this);
		RateController::Instance ()->AddSocket (pc);
		ConnectionManager::Instance ()->AddConnection (pc);

		initializeConnection (pc);
		TCI_->Connections_ << pc;

		TorrentPeer *tp = free.takeAt (qrand () % free.size ());
		free.removeAll (tp);
		tp->ConnectStart_ = QDateTime::currentDateTime ().toTime_t ();
		tp->LastVisited_ = tp->ConnectStart_;

		pc->SetPeer (tp);
		pc->connectToHost (tp->Address_, tp->Port_);
	}
}

QList<TorrentPeer*> TorrentClient::getWeighedFreePeers () const
{
	QList<TorrentPeer*> result;

	uint now = QDateTime::currentDateTime ().toTime_t ();
	QList<TorrentPeer*> freePeers;
	QMap<QString, int> connectionsPerPeer;
	int maxPerPeer = SettingsManager::Instance ()->GetMaxConnectionsPerPeer ();
	int revisitTime = SettingsManager::Instance ()->GetMinimumRevisitTime ();
	foreach (TorrentPeer *tp, TCI_->Peers_)
	{
		bool busy = false;
		foreach (PeerConnection *pc, TCI_->Connections_)
			if (pc->state () == PeerConnection::ConnectedState &&
				pc->peerAddress () == tp->Address_ &&
				pc->peerPort () == tp->Port_)
				if (++connectionsPerPeer[tp->Address_.toString ()] >= maxPerPeer)
				{
					busy = true;
					break;
				}

		if (!busy && (now - tp->LastVisited_) > static_cast<uint> (revisitTime))
			freePeers << tp;
	}

	if (freePeers.isEmpty ())
		return result;

	typedef QPair<int, TorrentPeer*> PointPair;
	QList<PointPair> points;
	foreach (TorrentPeer *tp, freePeers)
	{
		int p = 0;
		if (tp->Interesting_)
		{
			p += tp->NumComplete_;
			if (TCI_->State_ == StateSeeding)
				p = TCI_->PieceCount_ - p;
			if (!tp->ConnectStart_)
				p += TCI_->PieceCount_;

			if (tp->ConnectTime_ < 5)
				p += (TCI_->PieceCount_ * 10) / (5 - tp->ConnectTime_);
		}
		points << PointPair (p, tp);
	}

	qSort (points);

	QMultiMap<int, TorrentPeer*> pMap;
	int lowest = 0, lastIndex = 0;
	foreach (PointPair pp, points)
	{
		if (pp.first > lowest)
		{
			lowest = pp.first;
			++lastIndex;
		}
		pMap.insert (lastIndex, pp.second);
	}

	for (QMultiMap<int, TorrentPeer*>::ConstIterator i = pMap.constBegin (); i != pMap.constEnd (); ++i)
		for (int j = 0; j < i.key () + 1; ++j)
			result << (*i);

	return result;
}

void TorrentClient::setupOutgoingConnection ()
{
	PeerConnection *pc = qobject_cast<PeerConnection*> (sender ());

	foreach (TorrentPeer *tp, TCI_->Peers_)
		if (tp->Port_ == pc->peerPort () && tp->Address_ == pc->peerAddress ())
		{
			tp->ConnectTime_ = tp->LastVisited_ - tp->ConnectStart_;
			break;
		}

	pc->Initialize (TCI_->InfoHash_, TCI_->PieceCount_);
	pc->SendPieceList (TCI_->CompletedPieces_);

	emit peerInfoUpdated ();

	if (TCI_->State_ == StateSearching || TCI_->State_ == StateConnecting)
	{
		if (!TCI_->CompletedPieces_.count (true))
			TCI_->SetState (StateWarmingUp);
		else if (TCI_->IncompletePieces_.count (true) < SettingsManager::Instance ()->GetEndGamePieces () && TCI_->PendingPieces_.size () > TCI_->IncompletePieces_.count (true))
			TCI_->SetState (StateEndGame);
	}
}

void TorrentClient::initializeConnection (PeerConnection *pc)
{
	connect (pc, SIGNAL (connected ()), this, SLOT (setupOutgoingConnection ()));
	connect (pc, SIGNAL (disconnected ()), this, SLOT (removeClient ()));
	connect (pc, SIGNAL (error (QAbstractSocket::SocketError)), this, SLOT (removeClient ()));
	connect (pc, SIGNAL (piecesAvailable (const QBitArray&)), this, SLOT (peerPiecesAvailable (const QBitArray&)));
	connect (pc, SIGNAL (blockRequested (int, int, int)), this, SLOT (peerRequestsBlock (int, int, int)));
	connect (pc, SIGNAL (blockReceived (int, int, const QByteArray&)), this, SLOT (blockReceived (int, int, const QByteArray&)));
	connect (pc, SIGNAL (choked ()), this, SLOT (peerChoked ()));
	connect (pc, SIGNAL (unchoked ()), this, SLOT (peerUnchoked ()));
	connect (pc, SIGNAL (bytesWritten (qint64)), this, SLOT (peerConnectionBytesWritten (qint64)));
	connect (pc, SIGNAL (bytesReceived (qint64)), this, SLOT (peerConnectionBytesReceived (qint64)));
}

void TorrentClient::removeClient ()
{
	PeerConnection *client = qobject_cast<PeerConnection*> (sender ());

	if (client->GetPeer () && client->error () == QAbstractSocket::ConnectionRefusedError)
		TCI_->Peers_.removeAll (client->GetPeer ());

	RateController::Instance ()->RemoveSocket (client);
	TCI_->Connections_.removeAll (client);
	QMultiMap<PeerConnection*, TorrentPiece*>::Iterator i = TCI_->Payloads_.find (client);
	while (i != TCI_->Payloads_.end () && i.key () == client)
	{
		TorrentPiece *p = (*i);
		p->InProgress_ = false;
		p->Requested_.fill (false);
		i = TCI_->Payloads_.erase (i);
	}

	QMapIterator<int, PeerConnection*> j (TCI_->ReadIDs_);
	while (j.findNext (client))
		TCI_->ReadIDs_.remove (j.key ());

	disconnect (client, SIGNAL (disconnected ()), this, SLOT (removeClient ()));
	client->deleteLater ();
	ConnectionManager::Instance ()->RemoveConnection (client);

	emit peerInfoUpdated ();
	TCI_->CallPeerConnector ();
}

void TorrentClient::peerPiecesAvailable (const QBitArray& pieces)
{
	PeerConnection *pc = qobject_cast<PeerConnection*> (sender ());

	TorrentPeer *peer = 0;
	QList<TorrentPeer*>::Iterator i = TCI_->Peers_.begin ();
	while (i != TCI_->Peers_.end ())
	{
		if ((*i)->Address_ == pc->peerAddress () && (*i)->Port_ == pc->peerPort ())
		{
			peer = *i;
			break;
		}
		++i;
	}

	if (pieces.count (true) == TCI_->PieceCount_)
	{
		if (peer)
			peer->Seed_ = true;
		emit peerInfoUpdated ();
		if (TCI_->State_ == StateSeeding)
		{
			pc->abort ();
			return;
		}
		else
		{
			if (peer)
				peer->Interesting_ = true;
			if (!(pc->GetPeerConnectionState () & PeerConnection::AmInterested))
				pc->SendInterested ();

			TCI_->CallScheduler ();
			return;
		}
	}

	if (peer)
	{
		peer->Pieces_ = pieces;
		peer->NumComplete_ = pieces.count (true);
	}

	bool interested = false;
	int piecesSize = pieces.size ();
	for (int pindex = 0; pindex < piecesSize; ++i)
	{
		if (!pieces.testBit (pindex))
			continue;
		if (!TCI_->CompletedPieces_.testBit (pindex))
		{
			interested = true;
			if (!(pc->GetPeerConnectionState () & PeerConnection::AmInterested))
			{
				if (peer)
					peer->Interesting_ = true;
				pc->SendInterested ();
			}

			int inProgress = 0;
			for (QMultiMap<PeerConnection*, TorrentPiece*>::Iterator i = TCI_->Payloads_.find (pc); i != TCI_->Payloads_.end () && i.key () == pc; ++i)
				if (i.value ()->InProgress_)
					inProgress += i.value ()->Requested_.count (true);
			if (!inProgress)
				TCI_->CallScheduler ();
			break;
		}
	}
	if (!interested && (pc->GetPeerConnectionState () & PeerConnection::AmInterested))
	{
		if (peer)
			peer->Interesting_ = false;
		pc->SendNotInterested ();
	}
}

void TorrentClient::peerRequestsBlock (int index, int offset, int length)
{
	PeerConnection *pc = qobject_cast<PeerConnection*> (sender ());

	if (pc->GetPeerConnectionState () & PeerConnection::Choking)
		return;

	if (!TCI_->CompletedPieces_.testBit (index))
		return;

	TCI_->ReadIDs_.insert (TCI_->FileManager_.Read (index, offset, length), pc);
}

void TorrentClient::blockReceived (int index, int begin, const QByteArray& data)
{
	qDebug () << Q_FUNC_INFO;
	PeerConnection *client = qobject_cast<PeerConnection*> (sender ());

	if (data.size () == 0)
	{
		client->abort ();
		return;
	}
	
	int blockBit = begin / SettingsManager::Instance ()->GetBlockSize ();
	TorrentPiece *piece = TCI_->PendingPieces_ [index];
	if (!piece || piece->Completed_.testBit (blockBit))
	{
		requestMore (client);
		return;
	}

	if (TCI_->State_ == StateWarmingUp || TCI_->State_ == StateEndGame)
		for (QMultiMap<PeerConnection*, TorrentPiece*>::Iterator i = TCI_->Payloads_.begin (); i != TCI_->Payloads_.end (); ++i)
			if (i.key () != client &&
				i.value ()->Index_ == index &&
				i.key ()->GetInBlocks ().contains (Block (index, begin, data.size ())))
				i.key ()->SendCancel (index, begin, data.size ());

	if (TCI_->State_ != StateDownloading && TCI_->State_ != StateEndGame && TCI_->CompletedPieces_.count (true))
		TCI_->SetState (StateDownloading);

	TCI_->FileManager_.Write (index, begin, data);
	piece->Completed_.setBit (index);
	piece->Requested_.clearBit (index);

	if (!getBlocksLeftForPiece (piece))
	{
		TCI_->FileManager_.VerifyPiece (piece->Index_);
		QMultiMap<PeerConnection*, TorrentPiece*>::Iterator i = TCI_->Payloads_.begin ();
		while (i != TCI_->Payloads_.end ())
			if (!i.value () || i.value ()->Index_ == piece->Index_)
				i = TCI_->Payloads_.erase (i);
			else
				++i;
	}

	requestMore (client);
}

void TorrentClient::peerConnectionBytesWritten (qint64 bytes)
{
	if (!TCI_->TransferRateTimer_)
		TCI_->TransferRateTimer_ = startTimer (SettingsManager::Instance ()->GetRateControlTimerDelay ());

	TCI_->UploadRate_ [0] += bytes;
	TCI_->UploadedBytes_ += bytes;
	emit dataSent (bytes);
}

void TorrentClient::peerConnectionBytesReceived (qint64 bytes)
{
	if (!TCI_->TransferRateTimer_)
		TCI_->TransferRateTimer_ = startTimer (SettingsManager::Instance ()->GetRateControlTimerDelay ());

	TCI_->DownloadRate_ [0] += bytes;
	TCI_->DownloadedBytes_ += bytes;
	emit dataReceived (bytes);
}

int TorrentClient::getBlocksLeftForPiece (const TorrentPiece *piece) const
{
	int result = 0;

	int completed = piece->Completed_.size ();
	for (int i = 0; i < completed; ++i)
		if (!piece->Completed_.testBit (i))
			++result;

	return result;
}

void TorrentClient::scheduleUploads ()
{
	QList<PeerConnection*> allClients = TCI_->Connections_;
	QMultiMap<int, PeerConnection*> transferSpeeds;
	foreach (PeerConnection *pc, allClients)
	{
		if (pc->state () == QAbstractSocket::ConnectedState &&
			pc->GetAvailablePieces ().count (true) != TCI_->PieceCount_)
		{
			if (TCI_->State_ == StateSeeding)
				transferSpeeds.insert (pc->GetUploadSpeed (), pc);
			else
				transferSpeeds.insert (pc->GetDownloadSpeed (), pc);
		}
	}

	int maxUploaders = SettingsManager::Instance ()->GetMaxUploads ();
	QMapIterator<int, PeerConnection*> i (transferSpeeds);
	i.toBack ();
	while (i.hasPrevious ())
	{
		PeerConnection *client = i.previous ().value ();
		bool interested = (client->GetPeerConnectionState () & PeerConnection::PeerInterested);

		if (maxUploaders)
		{
			allClients.removeAll (client);
			if (client->GetPeerConnectionState () & PeerConnection::Choking)
				client->SendUnchoke ();
			--maxUploaders;
			continue;
		}

		if (!(client->GetPeerConnectionState () & PeerConnection::Choking))
		{
			if (!(qrand () % 10))
				client->abort ();
			else
				client->SendChoke ();
			allClients.removeAll (client);
		}

		if (!interested)
			allClients.removeAll (client);
	}

	if (!allClients.isEmpty ())
	{
		PeerConnection *pc = allClients [qrand () % allClients.size ()];
		if (pc->GetPeerConnectionState () & PeerConnection::Choking)
			pc->SendUnchoke ();
	}
}

void TorrentClient::scheduleDownloads ()
{
	TCI_->SchedulerCalled_ = false;

	if (TCI_->State_ == StateStopping || TCI_->State_ == StatePaused || TCI_->State_ == StateIdle)
		return;

	foreach (PeerConnection *pc, TCI_->Connections_)
		schedulePieceForClient (pc);
}

void TorrentClient::schedulePieceForClient (PeerConnection *client)
{
	if (client->state () != QAbstractSocket::ConnectedState || (client->GetPeerConnectionState () & PeerConnection::Choked))
		return;

	QList<int> currentPieces;
	bool someNot = false;
	TorrentPiece *lastPendingPiece = 0;

	for (QMultiMap<PeerConnection*, TorrentPiece*>::Iterator i = TCI_->Payloads_.find (client); i != TCI_->Payloads_.end () && i.key () == client; ++i)
	{
		lastPendingPiece = i.value ();
		if (lastPendingPiece->InProgress_)
			currentPieces << lastPendingPiece->Index_;
		else
			someNot = true;
	}

	if (client->GetInBlocks ().size () >= ((TCI_->State_ == StateEndGame || TCI_->State_ == StateWarmingUp) ?
		SettingsManager::Instance ()->GetMaxBlocksInMultiMode () :
		SettingsManager::Instance ()->GetMaxBlocksInProgress ()))
		return;

	if (!someNot || client->GetInBlocks ().size () > 0)
		lastPendingPiece = 0;
	TorrentPiece *piece = lastPendingPiece;

	if (TCI_->State_ == StateWarmingUp && TCI_->PendingPieces_.size () >= 4)
	{
		piece = TCI_->Payloads_.value (client);
		if (!piece)
		{
			QList<TorrentPiece*> values = TCI_->PendingPieces_.values ();
			piece = values [qrand () % values.size ()];
			piece->InProgress_ = true;
			TCI_->Payloads_.insert (client, piece);
		}
		if (piece->Completed_.count (false) == client->GetInBlocks ().size ())
			return;
	}

	if (!piece)
	{
		QBitArray incompleteAvailable = TCI_->IncompletePieces_;
		if (TCI_->State_ == StateEndGame && client->GetUploadSpeed () < SettingsManager::Instance ()->GetConsiderableUploadSpeed () || TCI_->State_ != StateWarmingUp)
			for (QMap<int, TorrentPiece*>::ConstIterator i = TCI_->PendingPieces_.constBegin (); i != TCI_->PendingPieces_.constEnd (); ++i)
				if (i.value ()->InProgress_)
					incompleteAvailable.clearBit (i.key ());

		incompleteAvailable &= client->GetAvailablePieces ();

		foreach (int i, currentPieces)
			incompleteAvailable.clearBit (i);

		if (!incompleteAvailable.count (true))
			return;

		QList<TorrentPiece*> partial;
		for (QMap<int, TorrentPiece*>::ConstIterator i = TCI_->PendingPieces_.constBegin (); i != TCI_->PendingPieces_.constEnd (); ++i)
			if (incompleteAvailable.testBit (i.key ()))
				if (!i.value ()->InProgress_ || TCI_->State_ == StateWarmingUp || TCI_->State_ == StateEndGame)
					partial << i.value ();
		if (!partial.isEmpty ())
			piece = partial.value (qrand () % partial.size ());

		if (!piece)
		{
			int pindex = 0;
			if (TCI_->State_ == StateWarmingUp || (qrand () % 4) == 0)
			{
				int *occ = new int [TCI_->PieceCount_];
				std::memset (occ, 0, TCI_->PieceCount_ * sizeof (int));
				foreach (PeerConnection *pc, TCI_->Connections_)
				{
					QBitArray pieces = pc->GetAvailablePieces ();
					int psize = pieces.size ();
					for (int i = 0; i < psize; ++i)
						occ [i] += pieces.testBit (i);
				}

				int numOcc = (TCI_->State_ == StateWarmingUp) ? 0 : 99999;
				QList<int> piecesReadyForDownload;
				for (int i = 0; i < TCI_->PieceCount_; ++i)
				{
					if (TCI_->State_ == StateWarmingUp)
						if (occ [i] >= numOcc &&
							incompleteAvailable.testBit (i))
						{
							if (occ [i] > numOcc)
								piecesReadyForDownload.clear ();
							piecesReadyForDownload.append (i);
							numOcc = occ [i];
						}
					else
					{
						if (occ [i] <= numOcc &&
							incompleteAvailable.testBit (i))
						{
							if (occ [i] < numOcc )
								piecesReadyForDownload.clear ();
							piecesReadyForDownload.append (i);
							numOcc = occ [i];
						}
					}
				}

				pindex = piecesReadyForDownload [qrand () % piecesReadyForDownload.size ()];
				delete [] occ;
			}
			else
			{
				QList<int> vals;
				int incompleteAvailableSize = incompleteAvailable.size ();
				for (int i = 0; i < incompleteAvailableSize; ++i)
					if (incompleteAvailable.testBit (i))
							vals << i;
				pindex = vals [qrand () % vals.size ()];
			}

			piece = new TorrentPiece;
			piece->Index_ = pindex;
			piece->Length_ = TCI_->FileManager_.GetPieceLengthAt (pindex);
			int numBlocks = piece->Length_ / SettingsManager::Instance ()->GetBlockSize ();
			if (piece->Length_ % SettingsManager::Instance ()->GetBlockSize ())
				++numBlocks;
			piece->Completed_.resize (numBlocks);
			piece->Requested_.resize (numBlocks);
			TCI_->PendingPieces_.insert (pindex, piece);
		}

		piece->InProgress_ = true;
		TCI_->Payloads_.insert (client, piece);
	}

	requestMore (client);
}

void TorrentClient::requestMore (PeerConnection *client)
{
	int numBlocksInProgress = client->GetInBlocks ().size ();
	QList<TorrentPiece*> piecesInProgress;
	for (QMultiMap<PeerConnection*, TorrentPiece*>::Iterator i = TCI_->Payloads_.begin (); i != TCI_->Payloads_.end () && i.key () == client; ++i)
	{
		TorrentPiece *p = i.value ();
		if (p->InProgress_ || TCI_->State_ == StateWarmingUp || TCI_->State_ == StateEndGame)
				piecesInProgress << p;
	}

	if (piecesInProgress.isEmpty () && TCI_->IncompletePieces_.count (true))
	{
		TCI_->CallScheduler ();
		return;
	}

	int maxip = ((TCI_->State_ == StateEndGame || TCI_->State_ == StateWarmingUp) ?
				 SettingsManager::Instance ()->GetMaxBlocksInMultiMode () :
				 SettingsManager::Instance ()->GetMaxBlocksInProgress ());
	if (numBlocksInProgress == maxip)
		return;

	foreach (TorrentPiece *piece, piecesInProgress)
	{
		numBlocksInProgress += requestBlocks (client, piece, maxip - numBlocksInProgress);
		if (numBlocksInProgress == maxip)
			break;
	}

	if (numBlocksInProgress < maxip && TCI_->State_ != StateWarmingUp)
		TCI_->CallScheduler ();
}

int TorrentClient::requestBlocks (PeerConnection *client, TorrentPiece *piece, int maxBlocks)
{
	QVector<int> bits;
	int completedBlocksSize = piece->Completed_.size ();
	for (int i = 0; i < completedBlocksSize; ++i)
		if (!piece->Completed_.testBit (i) && !piece->Requested_.testBit (i))
			bits << i;

	if (bits.size () == 0)
	{
		if (TCI_->State_ != StateWarmingUp && TCI_->State_ != StateEndGame)
			return 0;
		bits.clear ();
		for (int i = 0; i < completedBlocksSize; ++i)
			if (!piece->Completed_.testBit (i))
				bits << i;
	}

	if (TCI_->State_ == StateWarmingUp || TCI_->State_ == StateEndGame)
		for (int i = 0; i < bits.size (); ++i)
		{
			int a = qrand () % bits.size ();
			int b = qrand () % bits.size ();
			int tmp = bits [a];
			bits [a] = bits [b];
			bits [b] = tmp;
		}

	int blocks2Request = qMin<int> (maxBlocks, bits.size ());

	int blockSize = SettingsManager::Instance ()->GetBlockSize ();
	for (int i = 0; i < blocks2Request; ++i)
	{
		int bs = blockSize;
		if ((piece->Length_ % bs) && bits [i] == completedBlocksSize - 1)
			bs = piece->Length_ % blockSize;
		client->SendRequest (piece->Index_, bits [i] * blockSize, bs);
		piece->Requested_.setBit (bits [i]);
	}

	return blocks2Request;
}

void TorrentClient::peerChoked ()
{
	PeerConnection *pc = qobject_cast<PeerConnection*> (sender ());
	if (!pc)
		return;

	QMultiMap<PeerConnection*, TorrentPiece*>::Iterator i = TCI_->Payloads_.find (pc);
	while (i != TCI_->Payloads_.end () && i.key () == pc)
	{
		i.value ()->InProgress_ = false;
		i.value ()->Requested_.fill (false);
		i = TCI_->Payloads_.erase (i);
	}
}

void TorrentClient::peerUnchoked ()
{
	PeerConnection *pc = qobject_cast<PeerConnection*> (sender ());
	if (!pc)
		return;

	if (TCI_->State_ != StateSeeding)
		TCI_->CallScheduler ();
}

void TorrentClient::addToPeerList (const QList<TorrentPeer>& peerList)
{
	foreach (TorrentPeer peer, peerList)
	{
		if (peer.Address_ == TorrentServer::Instance ()->serverAddress () &&
			peer.Port_ == TorrentServer::Instance ()->serverPort ())
			continue;
		bool known = false;
		foreach (TorrentPeer *tp, TCI_->Peers_)
			if (tp->Address_ == peer.Address_ &&
				tp->Port_ == peer.Port_)
			{
				known = true;
				break;
			}
		if (!known)
		{
			TorrentPeer *np = new TorrentPeer;
			*np = peer;
			np->Interesting_ = false;
			np->Seed_ = false;
			np->LastVisited_ = 0;
			np->ConnectStart_ = 0;
			np->ConnectTime_ = 999999;
			np->Pieces_.resize (TCI_->PieceCount_);
			TCI_->Peers_ << np;
		}
	}

	int maxPeers = ConnectionManager::Instance ()->GetMaxConnections () * 3;
	if (TCI_->Peers_.size () > maxPeers)
	{
		QSet<TorrentPeer*> active;
		foreach (TorrentPeer *peer, TCI_->Peers_)
			foreach (PeerConnection *client, TCI_->Connections_)
				if (client->GetPeer () == peer && (client->GetDownloadSpeed () + client->GetUploadSpeed ()) > 1024)
					active << peer;

		QList<int> toRemove;
		for (int i = 0; i < TCI_->Peers_.size () && (TCI_->Peers_.size () - toRemove.size ()) > maxPeers; ++i)
			if (!active.contains (TCI_->Peers_ [i]))
				toRemove << i;
		QListIterator<int> tri (toRemove);
		tri.toBack ();
		while (tri.hasPrevious ())
			TCI_->Peers_.removeAt (tri.previous ());

		while (TCI_->Peers_.size () > maxPeers)
			/* delete */ TCI_->Peers_.takeFirst ();
	}

	if (TCI_->State_ != StatePaused && TCI_->State_ != StateIdle && TCI_->State_ != StateStopping)
	{
		if (TCI_->State_ == StateSearching || TCI_->State_ == StateWarmingUp)
			connectToPeers ();
		else
			TCI_->CallPeerConnector ();
	}
}

void TorrentClient::trackerStopped ()
{
	TCI_->SetState (StateIdle);
	emit stopped ();
}

void TorrentClient::updateProgress (int progress)
{
	if (progress == -1 && TCI_->PieceCount_ > 0)
	{
		int newProgress = (TCI_->CompletedPieces_.count (true) * 100) / TCI_->PieceCount_;
		if (TCI_->LastProgressValue_ != newProgress)
		{
			TCI_->LastProgressValue_ = newProgress;
			emit progressUpdated (newProgress);
		}
	}
	else if (TCI_->LastProgressValue_ != progress)
	{
		TCI_->LastProgressValue_ = progress;
		emit progressUpdated (progress);
	}
}

