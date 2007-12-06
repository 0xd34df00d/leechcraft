#include <QtCore>
#include "trackerclient.h"
#include "bencodeparser.h"
#include "torrentserver.h"
#include "torrentclient.h"
#include "connectionmanager.h"

namespace
{
	quint32 qntoh (quint32 source)
	{
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
		return source;
#else
		return 0 | 
			((source & 0x000000ff) << 24) |
			((source & 0x0000ff00) << 8) |
			((source & 0x00ff0000) >> 8) |
			((source & 0xff000000) >> 24);
#endif
	}
};

TrackerClient::TrackerClient (TorrentClient *tc, QObject *parent)
: QObject (parent)
, TorrentClient_ (tc)
, RequestInterval_ (5 * 60)
, RequestIntervalTimer_ (-1)
, Uploaded_ (0)
, Downloaded_ (0)
, Length_ (0)
, FirstTrackerRequest_ (true)
, LastTrackerRequest_ (false)
{
	connect (&Http_, SIGNAL (done (bool)), this, SLOT (httpRequestDone (bool)));
}

void TrackerClient::Start (const MetaInfo& mi)
{
	MetaInfo_ = mi;
	QTimer::singleShot (0, this, SLOT (fetchPeerList ()));

	if (MetaInfo_.GetForm () == MetaInfo::FormSingle)
	{
		Length_ = MetaInfo_.GetSingleFileInfo ().Length_;
	}
	else
	{
		QList<MetaInfoMultifile> files = MetaInfo_.GetMultiFilesInfo ();
		for (int i = 0; i < files.size (); ++i)
			Length_ += files [i].Length_;
	}
}

void TrackerClient::Stop ()
{
	LastTrackerRequest_ = true;
	Http_.abort ();
	fetchPeerList ();
}

qint64 TrackerClient::GetUploadCount () const
{
	return Uploaded_;
}

qint64 TrackerClient::GetDownloadCount () const
{
	return Downloaded_;
}

void TrackerClient::SetUploadCount (qint64 val)
{
	Uploaded_ = val;
}

void TrackerClient::SetDownloadCount (qint64 val)
{
	Downloaded_ = val;
}

void TrackerClient::timerEvent (QTimerEvent *e)
{
	if (e->timerId () == RequestIntervalTimer_)
	{
		if (Http_.state () == QHttp::Unconnected)
			fetchPeerList ();
	}
	else
		QObject::timerEvent (e);
}

void TrackerClient::fetchPeerList ()
{
	qDebug () << Q_FUNC_INFO;
	QUrl url (MetaInfo_.GetAnnounceURL ());

	QByteArray infoHash = TorrentClient_->GetInfoHash ();
	QByteArray encodedSum;
	for (int i = 0; i < infoHash.size (); ++i)
	{
		encodedSum += '%';
		encodedSum += QString::number (infoHash [i], 16).right (2).rightJustified (2, '0');
	}

	bool seeding = (TorrentClient_->GetState () == TorrentClient::StateSeeding);
	QByteArray query;
	query += url.path ().toLatin1 ();
	query += "?";
	query += "info_hash=" + encodedSum.toUpper ();
	query += "&peer_id=" + ConnectionManager::Instance ()->GetClientID ();
	query += "&port=" + QByteArray::number (TorrentServer::Instance ()->serverPort ());
	query += "&uploaded=" + QByteArray::number (Uploaded_);
	query += "&downloaded=" + QByteArray::number (Downloaded_);
	query += "&left=" + QByteArray::number (seeding ? 0 : qMax<int> (0, Length_ - Downloaded_));
	query += "&compact=1";
	if (seeding)
		query += "&event=completed";
	else if (FirstTrackerRequest_)
		query += "&event=started";
	else if (LastTrackerRequest_)
		query += "&event=stopped";

	if (!TrackerID_.isEmpty ())
		query += "&trackerid=" + TrackerID_;

	Http_.setHost (url.host (), url.port () == -1 ? 80 : url.port ());
	if (!url.userName ().isEmpty ())
		Http_.setUser (url.userName (), url.password ());


	Http_.get (query);
}

void TrackerClient::httpRequestDone (bool error)
{
	if (LastTrackerRequest_)
	{
		emit stopped ();
		return;
	}

	if (error)
	{
		emit connectionError (Http_.error ());
		return;
	}
	
	if (Http_.lastResponse ().statusCode () != 200)
		qDebug () << Q_FUNC_INFO << "status code isn't 200, but" << Http_.lastResponse ().statusCode ();

	QByteArray response = Http_.readAll ();
	Http_.abort ();

	BencodeParser parser;
	if (!parser.Parse (response))
	{
		qWarning ("Error parsing bencode response from tracker: %s", qPrintable (parser.GetErrorString ()));
		Http_.abort ();
		return;
	}

	Dictionary_t dict = parser.GetParsed ();

	if (dict.contains ("failure reason"))
	{
		emit failure (QString::fromUtf8 (dict ["failure reason"].toByteArray ()));
		return;
	}
	if (dict.contains ("warning message"))
		emit warning (QString::fromUtf8 (dict ["warning message"].toByteArray ()));
	if (dict.contains ("tracker id"))
		TrackerID_ = dict ["tracker id"].toByteArray ();
	if (dict.contains ("interval"))
	{
		if (RequestIntervalTimer_ != -1)
			killTimer (RequestIntervalTimer_);
		RequestIntervalTimer_ = startTimer (dict ["interval"].toInt () * 1000);
	}
	if (dict.contains ("peers"))
	{
		Peers_.clear ();
		QVariant peerEntry = dict ["peers"];

		if (peerEntry.type () == QVariant::List)
		{
			QList<QVariant> peerTmp = peerEntry.toList ();
			for (int i = 0; i < peerTmp.size (); ++i)
			{
				TorrentPeer tmp;
				QMap<QByteArray, QVariant> peer = peerTmp.at (i).value<QMap<QByteArray, QVariant> > ();
				tmp.ID_ = QString::fromUtf8 (peer ["peer id"].toByteArray ());
				tmp.Address_.setAddress (QString::fromUtf8 (peer ["ip"].toByteArray ()));
				tmp.Port_ = peer ["port"].toInt ();
				Peers_ << tmp;
			}
		}
		else
		{
			QByteArray peerTmp = peerEntry.toByteArray ();
			for (int i = 0; i < peerTmp.size (); i += 6)
			{
				TorrentPeer tmp;
				uchar *data = reinterpret_cast<uchar*> (const_cast<char*> (peerTmp.constData ())) + i;
				tmp.Port_ = (static_cast<int> (data [4]) << 8) + data [5];
				uint ip = 0;
				ip += static_cast<uint> (data [0]) << 24;
				ip += static_cast<uint> (data [1]) << 16;
				ip += static_cast<uint> (data [2]) << 8;
				ip += static_cast<uint> (data [3]);
				tmp.Address_.setAddress (ip);
				Peers_ << tmp;
			}
		}
		emit peerListUpdated (Peers_);
	}
}














