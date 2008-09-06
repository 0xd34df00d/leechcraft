#define TORRENT_MAX_ALERT_TYPES 25
#include "core.h"
#include <QFile>
#include <QProgressDialog>
#include <QDir>
#include <QFileInfo>
#include <QTimerEvent>
#include <QSettings>
#include <QTimer>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>
#include <QtDebug>
#include <QThreadPool>
#include <QApplication>
#include <QRunnable>
#include <memory>
#include <libtorrent/alert.hpp>
#include <libtorrent/bencode.hpp>
#include <libtorrent/entry.hpp>
#include <libtorrent/create_torrent.hpp>
#include <libtorrent/extensions/metadata_transfer.hpp>
#include <libtorrent/extensions/ut_metadata.hpp>
#include <libtorrent/extensions/ut_pex.hpp>
#include <libtorrent/extensions/smart_ban.hpp>
#include <libtorrent/file_pool.hpp>
#include <libtorrent/hasher.hpp>
#include <libtorrent/storage.hpp>
#include <libtorrent/file.hpp>
#include <libtorrent/alert_types.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>
#include <plugininterface/proxy.h>
#include <plugininterface/tagscompletionmodel.h>
#include "xmlsettingsmanager.h"
#include "piecesmodel.h"
#include "peersmodel.h"
#include "torrentfilesmodel.h"
#include "torrentplugin.h"
#include "representationmodel.h"

Core* Core::Instance ()
{
	static Core core;
	return &core;
}

Core::Core ()
: CurrentTorrent_ (-1)
, SettingsSaveTimer_ (new QTimer ())
, FinishedTimer_ (new QTimer ())
, WarningWatchdog_ (new QTimer ())
, ScrapeTimer_ (new QTimer ())
, PeersModel_ (new PeersModel ())
, PiecesModel_ (new PiecesModel ())
, TagsCompletionModel_ (new TagsCompletionModel)
, TorrentFilesModel_ (new TorrentFilesModel (false))
{
	for (quint16 i = 0; i < 65535; ++i)
		IDPool_.push_back (i);
}

Core::~Core ()
{
}

void Core::DoDelayedInit ()
{
    try
    {
		QString peerIDstring = XmlSettingsManager::Instance ()->
			property ("PeerIDString").toString ();
		QString ver = XmlSettingsManager::Instance ()->
			property ("PeerIDVersion").toString ();
        Session_ = new libtorrent::session (libtorrent::fingerprint
				(peerIDstring.toLatin1 ().constData (),
				 ver.at (0).digitValue (),
				 ver.at (1).digitValue (), 
				 ver.at (2).digitValue (),
				 ver.at (3).digitValue ()));
        QList<QVariant> ports = XmlSettingsManager::Instance ()->property ("TCPPortRange").toList ();
        Session_->listen_on (std::make_pair (ports.at (0).toInt (), ports.at (1).toInt ()));
        Session_->add_extension (&libtorrent::create_metadata_plugin);
        Session_->add_extension (&libtorrent::create_ut_pex_plugin);
		Session_->add_extension (&libtorrent::create_ut_metadata_plugin);
		Session_->add_extension (&libtorrent::create_smart_ban_plugin);
        if (XmlSettingsManager::Instance ()->property ("DHTEnabled").toBool ())
            Session_->start_dht (libtorrent::entry ());
        Session_->set_max_uploads (XmlSettingsManager::Instance ()->property ("MaxUploads").toInt ());
        Session_->set_max_connections (XmlSettingsManager::Instance ()->property ("MaxConnections").toInt ());
        Session_->start_lsd ();
        Session_->start_upnp ();
        Session_->start_natpmp ();
        setProxySettings ();
        setGeneralSettings ();
        setDHTSettings ();
    }
    catch (...)
    {
        qWarning () << "Seems like address is already in use.";
    }

    Headers_ << tr ("Name")
		<< tr ("State")
		<< tr ("Progress")
		<< tr ("Drate")
		<< tr ("Urate")
		<< tr ("Uploaded")
		<< tr ("Rating")
		<< tr ("Seeds/peers")
		<< tr ("Remaining");

    ReadSettings ();
    InterfaceUpdateTimer_ = startTimer (1000);
	connect (SettingsSaveTimer_.get (),
			SIGNAL (timeout ()),
			this,
			SLOT (writeSettings ()));
	SettingsSaveTimer_->start (XmlSettingsManager::Instance ()->
			property ("AutosaveInterval").toInt () * 1000);

	connect (FinishedTimer_.get (),
			SIGNAL (timeout ()),
			this,
			SLOT (checkFinished ()));
	FinishedTimer_->start (100);

	connect (WarningWatchdog_.get (),
			SIGNAL (timeout ()),
			this,
			SLOT (queryLibtorrentForWarnings ()));
	WarningWatchdog_->start (100);

	connect (ScrapeTimer_.get (),
			SIGNAL (timeout ()),
			this,
			SLOT (scrape ()));
	if (XmlSettingsManager::Instance ()->property ("ScrapeEnabled").toBool ())
		ScrapeTimer_->start (XmlSettingsManager::Instance ()->
				property ("ScrapeInterval").toInt () * 1000);

    ManipulateSettings ();
}

void Core::Release ()
{
    writeSettings ();
    killTimer (InterfaceUpdateTimer_);

	SettingsSaveTimer_.reset ();
	FinishedTimer_.reset ();
	WarningWatchdog_.reset ();
	ScrapeTimer_.reset ();
	PiecesModel_.reset ();
	PeersModel_.reset ();
	TagsCompletionModel_.reset ();
	TorrentFilesModel_.reset ();

	QObjectList kids = children ();
	for (int i = 0; i < kids.size (); ++i)
	{
		delete kids.at (i);
		kids [i] = 0;
	}

    Session_->stop_dht ();
	delete Session_;
	Session_ = 0;
}

PiecesModel* Core::GetPiecesModel ()
{
    return PiecesModel_.get ();
}

void Core::ClearPieces ()
{
    PiecesModel_->Clear ();
}

void Core::UpdatePieces ()
{
    if (!CheckValidity (CurrentTorrent_))
	{
		ClearPieces ();
        return;
	}

    std::vector<libtorrent::partial_piece_info> queue;
    Handles_.at (CurrentTorrent_).Handle_.get_download_queue (queue);
    PiecesModel_->Update (queue);
}

PeersModel* Core::GetPeersModel ()
{
    return PeersModel_.get ();
}

void Core::ClearPeers ()
{
    PeersModel_->Clear ();
}

void Core::UpdatePeers ()
{
    if (!CheckValidity (CurrentTorrent_))
	{
		ClearPeers ();
        return;
	}

    PeersModel_->Update (GetPeers (), CurrentTorrent_);
}

TorrentFilesModel* Core::GetTorrentFilesModel ()
{
	return TorrentFilesModel_.get ();
}

void Core::ClearFiles ()
{
	TorrentFilesModel_->Clear ();
}

void Core::UpdateFiles ()
{
	if (!CheckValidity (CurrentTorrent_))
	{
		ClearFiles ();
		return;
	}

	TorrentFilesModel_->UpdateFiles (GetTorrentFiles ());
}

void Core::ResetFiles ()
{
	if (!CheckValidity (CurrentTorrent_))
	{
		ClearFiles ();
		return;
	}

	TorrentFilesModel_->ResetFiles (GetTorrentFiles ());
}

int Core::columnCount (const QModelIndex&) const
{
    return Headers_.size ();
}

QVariant Core::data (const QModelIndex& index, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant ();

    int row = index.row (),
        column = index.column ();

    libtorrent::torrent_handle h = Handles_.at (row).Handle_;
    if (!h.is_valid ())
    {
        emit const_cast<Core*> (this)->error (tr ("%1: for row %2 torrent handle is invalid").arg (Q_FUNC_INFO).arg (row));
        return QVariant ();
    }

    libtorrent::torrent_status status = h.status ();
    libtorrent::torrent_info info = h.get_torrent_info ();
    switch (column)
    {
        case ColumnName:
            return QString::fromUtf8 (h.name ().c_str ());
        case ColumnState:
            if (Handles_.at (row).State_ == TSWaiting2Download ||
                Handles_.at (row).State_ == TSWaiting2Seed)
                return tr ("Waiting in queue");
            else
                return status.paused ? tr ("Idle") : GetStringForState (status.state);
        case ColumnProgress:
            return QString ("%1% (%2 of %3)")
				.arg (status.progress * 100, 0, 'f', 2)
				.arg (Proxy::Instance ()->MakePrettySize (status.total_done))
				.arg (Proxy::Instance ()->MakePrettySize (status.total_wanted));
        case ColumnDSpeed:
            return Proxy::Instance ()->MakePrettySize (status.download_payload_rate) + tr ("/s");
        case ColumnUSpeed:
            return Proxy::Instance ()->MakePrettySize (status.upload_payload_rate) + tr ("/s");
        case ColumnUploaded:
            return Proxy::Instance ()->MakePrettySize (status.total_payload_upload + Handles_.at (row).UploadedBefore_);
        case ColumnRating:
            return QString::number (static_cast<float> (status.total_payload_upload + Handles_.at (row).UploadedBefore_) / status.total_payload_download, 'g', 2);
        case ColumnSP:
			{
				QString result = QString ("%1/%2 (%3/%4)")
					.arg (status.num_seeds)
					.arg (status.num_peers)
					.arg (status.list_seeds)
					.arg (status.list_peers);

				if (status.num_complete != -1 && status.num_incomplete != -1)
					result += QString (" [%1/%2]")
						.arg (status.num_complete)
						.arg (status.num_incomplete);
				return result;
			}
        case ColumnRemaining:
            return Proxy::Instance ()->MakeTimeFromLong ((status.total_wanted - status.total_wanted_done) / status.download_payload_rate).toString ();
        default:
            return QVariant ();
    }
}

Qt::ItemFlags Core::flags (const QModelIndex&) const
{
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

bool Core::hasChildren (const QModelIndex& index) const
{
    return !index.isValid ();
}

QModelIndex Core::index (int row, int column, const QModelIndex&) const
{
    if (!hasIndex (row, column))
        return QModelIndex ();

    return createIndex (row, column);
} 

QVariant Core::headerData (int column, Qt::Orientation orient, int role) const
{
    if (orient == Qt::Vertical)
        return QVariant ();

    if (role != Qt::DisplayRole)
        return QVariant ();

    return Headers_ [column];
}

QModelIndex Core::parent (const QModelIndex&) const
{
    return QModelIndex ();
}

int Core::rowCount (const QModelIndex& index) const
{
    if (index.isValid ())
        return 0;

    return Handles_.size ();
}

libtorrent::torrent_info Core::GetTorrentInfo (const QString& filename)
{
    QFile file (filename);
    if (!file.open (QIODevice::ReadOnly))
    {
        emit error (tr ("Could not open file %1 for read: %2").arg (filename).arg (file.errorString ()));
        return libtorrent::torrent_info (libtorrent::sha1_hash ());
    }
    return GetTorrentInfo (file.readAll ());
}

libtorrent::torrent_info Core::GetTorrentInfo (const QByteArray& data)
{
    try
    {
        libtorrent::torrent_info result (data.constData (), data.size ());
        return result;
    }
    catch (const libtorrent::invalid_encoding& e)
    {
        emit error (tr ("Bad bencoding in torrent file"));
        return libtorrent::torrent_info (libtorrent::sha1_hash ());
    }
    catch (const libtorrent::invalid_torrent_file& e)
    {
        emit error (tr ("Invalid torrent file"));
        return libtorrent::torrent_info (libtorrent::sha1_hash ());
    }
	catch (...)
	{
		emit error (tr ("General torrent parsing error"));
		return libtorrent::torrent_info (libtorrent::sha1_hash ());
	}
}

bool Core::IsValidTorrent (const QByteArray& torrentData) const
{
    try
    {
        libtorrent::torrent_info result (torrentData.constData (), torrentData.size ());
    }
    catch (...)
    {
        return false;
    }
    return true;
}

TorrentInfo Core::GetTorrentStats () const
{
    if (!CheckValidity (CurrentTorrent_))
        return TorrentInfo ();

    libtorrent::torrent_handle handle = Handles_.at (CurrentTorrent_).Handle_;
    libtorrent::torrent_status status = handle.status ();
    libtorrent::torrent_info info = handle.get_torrent_info ();

    TorrentInfo result;
    result.Tracker_ = QString::fromStdString (status.current_tracker).left (100);
    result.State_ = status.paused ? tr ("Idle") : GetStringForState (status.state);
    result.Downloaded_ = status.total_done;
    result.Uploaded_ = status.total_payload_upload + Handles_.at (CurrentTorrent_).UploadedBefore_;
    result.TotalSize_ = status.total_wanted;
    result.FailedSize_ = status.total_failed_bytes;
	result.RedundantBytes_ = status.total_redundant_bytes;
    result.DHTNodesCount_ = info.nodes ().size ();
    result.TotalPieces_ = info.num_pieces ();
    result.DownloadedPieces_ = status.num_pieces;
    result.DownloadRate_ = status.download_rate;
    result.UploadRate_ = status.upload_rate;
    result.ConnectedPeers_ = status.num_peers;
    result.ConnectedSeeds_ = status.num_seeds;
	result.PeersInList_ = status.list_peers;
	result.SeedsInList_ = status.list_seeds;
	result.PeersInSwarm_ = status.num_incomplete;
	result.SeedsInSwarm_ = status.num_complete;
	result.ConnectCandidates_ = status.connect_candidates;
	result.BlockSize_ = status.block_size;
    result.PieceSize_ = info.piece_length ();
    result.Progress_ = status.progress;
    result.DistributedCopies_ = status.distributed_copies;
	result.UpBandwidthQueue_ = status.up_bandwidth_queue;
	result.DownBandwidthQueue_ = status.down_bandwidth_queue;
	result.LastScrape_ = status.last_scrape;
	result.ActiveTime_ = status.active_time;
	result.SeedingTime_ = status.seeding_time;
	result.SeedRank_ = status.seed_rank;
    result.NextAnnounce_ = QTime (status.next_announce.hours (),
                                  status.next_announce.minutes (),
                                  status.next_announce.seconds ());
    result.AnnounceInterval_ = QTime (status.announce_interval.hours (),
                                      status.announce_interval.minutes (),
                                      status.announce_interval.seconds ());
    result.Pieces_ = status.pieces;
    return result;
}

libtorrent::bitfield Core::GetLocalPieces () const
{
    if (!CheckValidity (CurrentTorrent_))
        return 0;
    return Handles_.at (CurrentTorrent_).Handle_.status ().pieces;
}

OverallStats Core::GetOverallStats () const
{
    OverallStats result;
    libtorrent::session_status status = Session_->status ();

    result.ListenPort_ = Session_->listen_port ();
    result.NumUploads_ = Session_->num_uploads ();
    result.NumConnections_ = Session_->num_connections ();
    result.SessionUpload_ = status.total_upload;
    result.SessionDownload_ = status.total_download;
    result.UploadRate_ = status.upload_rate;
    result.DownloadRate_ = status.download_rate;
    result.NumPeers_ = status.num_peers;
    result.NumDHTNodes_ = status.dht_nodes;
    result.NumGlobalDHTNodes_ = status.dht_global_nodes;
    result.NumDHTTorrents_ = status.dht_torrents;
	result.TotalFailedData_ = status.total_failed_bytes;
	result.TotalRedundantData_ = status.total_redundant_bytes;
    return result;
}

libtorrent::cache_status Core::GetCacheStats () const
{
	return Session_->get_cache_status ();
}

QList<FileInfo> Core::GetTorrentFiles () const
{
    if (!CheckValidity (CurrentTorrent_))
        return QList<FileInfo> ();

    QList<FileInfo> result;
    libtorrent::torrent_handle handle = Handles_.at (CurrentTorrent_).Handle_;
    libtorrent::torrent_info info = handle.get_torrent_info ();
	std::vector<libtorrent::size_type> prbytes;
    handle.file_progress (prbytes);
    for (libtorrent::torrent_info::file_iterator i = info.begin_files (); i != info.end_files (); ++i)
    {
        FileInfo fi;
        fi.Path_ = i->path;
        fi.Size_ = i->size;
        fi.Priority_ = Handles_.at (CurrentTorrent_).FilePriorities_.at (i - info.begin_files ());
        fi.Progress_ = static_cast<float> (prbytes.at (i - info.begin_files ())) / 
			static_cast<float> (fi.Size_);
        result << fi;
    }

    return result;
}

QList<PeerInfo> Core::GetPeers () const
{
    if (!CheckValidity (CurrentTorrent_))
        return QList<PeerInfo> ();

    QList<PeerInfo> result;
    std::vector<libtorrent::peer_info> peerInfos;
    Handles_.at (CurrentTorrent_).Handle_.get_peer_info (peerInfos);

    for (size_t i = 0; i < peerInfos.size (); ++i)
    {
        libtorrent::peer_info pi = peerInfos [i];
        PeerInfo ppi;
        ppi.IP_ = QString::fromStdString (pi.ip.address ().to_string ());
        ppi.DSpeed_ = pi.down_speed;
        ppi.USpeed_ = pi.up_speed;
        ppi.Downloaded_ = pi.total_download;
        ppi.Uploaded_ = pi.total_upload;
        ppi.Client_ = QString::fromUtf8 (pi.client.c_str ());
        ppi.Pieces_ = pi.pieces;
        ppi.LoadBalancing_ = pi.load_balancing;
        ppi.LastActive_ = QTime (0, 0, 0);
        ppi.LastActive_.addMSecs (libtorrent::total_milliseconds (pi.last_active));
        ppi.Hashfails_ = pi.num_hashfails;
        ppi.Failcount_ = pi.failcount;
        ppi.DownloadingPiece_ = pi.downloading_piece_index;
        ppi.DownloadingBlock_ = pi.downloading_block_index;
        ppi.DownloadingProgress_ = pi.downloading_progress;
        ppi.DownloadingTotal_ = pi.downloading_total;
        result << ppi;
    }

    return result;
}

QStringList Core::GetTagsForIndex (int torrent) const
{
	if (torrent != -1)
		return GetTagsForIndexImpl (torrent);
	else
		return GetTagsForIndexImpl (CurrentTorrent_);
}

void Core::UpdateTags (const QStringList& tags, int torrent)
{
	if (torrent != -1)
		UpdateTagsImpl (tags, torrent);
	else
        UpdateTagsImpl (tags, CurrentTorrent_);
}

TagsCompletionModel* Core::GetTagsCompletionModel () const
{
    return TagsCompletionModel_.get ();
}

int Core::AddFile (const QString& filename,
		const QString& path,
		const QStringList& tags,
		const QVector<bool>& files,
		LeechCraft::TaskParameters params)
{
    if (!QFileInfo (filename).exists () || !QFileInfo (filename).isReadable ())
    {
        emit error (tr ("File %1 doesn't exist or could not be read").arg (filename));
        return -1;
    }

    libtorrent::torrent_handle handle;
    try
    {
		boost::intrusive_ptr<libtorrent::torrent_info> tinfo (
				new libtorrent::torrent_info (GetTorrentInfo (filename))
				);
		handle = Session_->add_torrent (tinfo,
				boost::filesystem::path (path.toStdString ()),
				libtorrent::entry (),
				libtorrent::storage_mode_allocate,
				!(params & LeechCraft::Autostart));
    }
    catch (const libtorrent::duplicate_torrent& e)
    {
        emit error (tr ("The torrent %1 with save path %2 already exists in the session").arg (filename).arg (path));
        return -1;
    }
    catch (const libtorrent::invalid_encoding& e)
    {
        emit error (tr ("Bad bencoding in torrent file"));
        return -1;
    }
    catch (const libtorrent::invalid_torrent_file& e)
    {
        emit error (tr ("Invalid torrent file"));
        return -1;
    }
    catch (const std::runtime_error& e)
    {
        emit error (tr ("Runtime error"));
        return -1;
    }

    std::vector<int> priorities;
    priorities.resize (handle.get_torrent_info ().num_files ());
    for (size_t i = 0; i < priorities.size (); ++i)
        priorities [i] = 1;

    if (files.size ())
    {
        for (int i = 0; i < files.size (); ++i)
            priorities [i] = files [i];

        handle.prioritize_files (priorities);
    }
    QFile file (filename);
    file.open (QIODevice::ReadOnly);
    QByteArray contents = file.readAll ();
    file.close ();

    beginInsertRows (QModelIndex (), Handles_.size (), Handles_.size ());
    TorrentStruct tmp = { 0, priorities, handle, contents, QFileInfo (filename).fileName (), TSIdle };
    tmp.Tags_ = tags;
	tmp.Parameters_ = params;
	tmp.ID_ = IDPool_.front ();
	IDPool_.pop_front ();
    Handles_.append (tmp);
    endInsertRows ();
    TagsCompletionModel_->UpdateTags (tags);

    QTimer::singleShot (3000, this, SLOT (writeSettings ()));
	return tmp.ID_;
}

void Core::RemoveTorrent (int pos)
{
    if (!CheckValidity (pos))
        return;

    Session_->remove_torrent (Handles_.at (pos).Handle_);
	int id = Handles_.at (pos).ID_;
    beginRemoveRows (QModelIndex (), pos, pos);
    Handles_.removeAt (pos);
    endRemoveRows ();
	IDPool_.push_front (id);

    QTimer::singleShot (3000, this, SLOT (writeSettings ()));
	emit taskRemoved (id);

	CheckUploadQueue ();
	CheckDownloadQueue ();
}

void Core::PauseTorrent (int pos)
{
    if (!CheckValidity (pos))
        return;

    Handles_.at (pos).Handle_.pause ();
    checkFinished ();
}

void Core::ResumeTorrent (int pos)
{
    if (!CheckValidity (pos))
        return;

    Handles_.at (pos).Handle_.resume ();
    Handles_ [pos].State_ = TSIdle;
    checkFinished ();
}

void Core::ForceReannounce (int pos)
{
    if (!CheckValidity (pos))
        return;

    try
    {
        Handles_.at (pos).Handle_.force_reannounce ();
    }
    catch (const libtorrent::invalid_handle&)
    {
        emit error (tr ("Torrent %1 could not be reannounced at the moment, try again later.").arg (pos));
    }
}

void Core::ForceRecheck (int pos)
{
	if (!CheckValidity (pos))
		return;
	
	Handles_.at (pos).Handle_.force_recheck ();
}

void Core::SetOverallDownloadRate (int val)
{
    Session_->set_download_rate_limit (val == 0 ? -1 : val * 1024);
    XmlSettingsManager::Instance ()->setProperty ("DownloadRateLimit", val);
}

void Core::SetOverallUploadRate (int val)
{
    Session_->set_upload_rate_limit (val == 0 ? -1 : val * 1024);
    XmlSettingsManager::Instance ()->setProperty ("UploadRateLimit", val);
}

void Core::SetMaxDownloadingTorrents (int val)
{
    XmlSettingsManager::Instance ()->setProperty ("MaxDownloadingTorrents", val);
    if (val)
    {
        int difference = val - GetCurrentlyDownloading ();
        if (difference > 0)
        {
            for (int i = 0; i < Handles_.size (); ++i)
            {
                if (Handles_.at (i).State_ == TSWaiting2Download)
                {
                    ResumeTorrent (i);
                    --difference;
                }
                if (!difference)
                    break;
            }
        }
        else if (difference < 0)
        {
            for (int i = 0; i < Handles_.size (); ++i)
            {
                if (Handles_.at (i).State_ == TSDownloading)
                {
                    Handles_.at (i).Handle_.pause ();
                    Handles_ [i].State_ = TSWaiting2Download;
                    ++difference;
                }
                if (!difference)
                    break;
            }
        }
    }
    else
        for (int i = 0; i < Handles_.size (); ++i)
            if (Handles_.at (i).State_ == TSWaiting2Download)
                ResumeTorrent (i);
}

void Core::SetMaxUploadingTorrents (int val)
{
    XmlSettingsManager::Instance ()->setProperty ("MaxUploadingTorrents", val);
    if (val)
    {
        int difference = val - GetCurrentlySeeding ();
        if (difference > 0)
        {
            for (int i = 0; i < Handles_.size (); ++i)
            {
                if (Handles_.at (i).State_ == TSWaiting2Seed)
                {
                    ResumeTorrent (i);
                    --difference;
                }
                if (!difference)
                    break;
            }
        }
        else if (difference < 0)
        {
            for (int i = 0; i < Handles_.size (); ++i)
            {
                if (Handles_.at (i).State_ == TSSeeding)
                {
                    Handles_.at (i).Handle_.pause ();
                    Handles_ [i].State_ = TSWaiting2Seed;
                    ++difference;
                }
                if (!difference)
                    break;
            }
        }
    }
    else
        for (int i = 0; i < Handles_.size (); ++i)
            if (Handles_.at (i).State_ == TSWaiting2Seed)
                ResumeTorrent (i);
}

void Core::SetDesiredRating (double val)
{
    for (int i = 0; i < Handles_.size (); ++i)
    {
        if (!CheckValidity (i))
            continue;

        Handles_.at (i).Handle_.set_ratio (val ? 1/val : 0);
    }

    XmlSettingsManager::Instance ()->setProperty ("DesiredRating", val);
}

int Core::GetOverallDownloadRate () const
{
    return XmlSettingsManager::Instance ()->property ("DownloadRateLimit").toInt ();
}

int Core::GetOverallUploadRate () const
{
    return XmlSettingsManager::Instance ()->property ("UploadRateLimit").toInt ();
}

int Core::GetMaxDownloadingTorrents () const
{
    return XmlSettingsManager::Instance ()->property ("MaxDownloadingTorrents").toInt ();
}

int Core::GetMaxUploadingTorrents () const
{
    return XmlSettingsManager::Instance ()->property ("MaxUploadingTorrents").toInt ();
}

double Core::GetDesiredRating () const
{
    return XmlSettingsManager::Instance ()->property ("DesiredRating").toInt ();
}

void Core::SetTorrentDownloadRate (int val)
{
    if (CheckValidity (CurrentTorrent_))
        Handles_.at (CurrentTorrent_).Handle_.set_download_limit (val == 0 ? -1 : val * 1024);
}

void Core::SetTorrentUploadRate (int val)
{
    if (CheckValidity (CurrentTorrent_))
        Handles_.at (CurrentTorrent_).Handle_.set_upload_limit (val == 0 ? -1 : val * 1024);
}

void Core::SetTorrentDesiredRating (double val)
{
    if (CheckValidity (CurrentTorrent_))
    {
        Handles_.at (CurrentTorrent_).Handle_.set_ratio (val ? 1/val : 0);
        Handles_ [CurrentTorrent_].Ratio_ = val;
    }
}

int Core::GetTorrentDownloadRate () const
{
    if (CheckValidity (CurrentTorrent_))
        return Handles_.at (CurrentTorrent_).Handle_.download_limit () / 1024;
    else
        return -1;
}

int Core::GetTorrentUploadRate () const
{
    if (CheckValidity (CurrentTorrent_))
        return Handles_.at (CurrentTorrent_).Handle_.upload_limit () / 1024;
    else
        return -1;
}

double Core::GetTorrentDesiredRating () const
{
    if (CheckValidity (CurrentTorrent_))
        return Handles_.at (CurrentTorrent_).Ratio_;
    else
        return -1;
}

void Core::SetFilePriority (int file, int priority)
{
    if (!CheckValidity (CurrentTorrent_))
        return;

    if (priority > 7)
        priority = 7;
    else if (priority < 0)
        priority = 0;

    try
    {
        Handles_ [CurrentTorrent_].FilePriorities_.at (file) = priority;
        Handles_.at (CurrentTorrent_).Handle_.prioritize_files (Handles_.at (CurrentTorrent_).FilePriorities_);
    }
    catch (...)
    {
        qWarning () << Q_FUNC_INFO << QString ("index for torrent %1, file %2 is out of bounds").arg (CurrentTorrent_).arg (file);
    }
}

int Core::GetFilePriority (int file) const
{
    if (!CheckValidity (CurrentTorrent_))
        return -1;

    return Handles_.at (CurrentTorrent_).FilePriorities_ [file];
}

QStringList Core::GetTrackers () const
{
    if (!CheckValidity (CurrentTorrent_))
        return QStringList ();

    std::vector<libtorrent::announce_entry> an = Handles_.at (CurrentTorrent_).Handle_.trackers ();
    QStringList result;
    for (size_t i = 0; i < an.size (); ++i)
        result.append (QString::fromStdString (an [i].url));
    return result;
}

QStringList Core::GetTrackers (int row) const
{
	int old = CurrentTorrent_;
	CurrentTorrent_ = row;
	GetTrackers ();
	CurrentTorrent_ = old;
}

void Core::SetTrackers (const QStringList& trackers)
{
    if (!CheckValidity (CurrentTorrent_))
        return;

    std::vector<libtorrent::announce_entry> announces;
    for (int i = 0; i < trackers.size (); ++i)
        announces.push_back (libtorrent::announce_entry (trackers.at (i).toStdString ()));
    Handles_ [CurrentTorrent_].Handle_.replace_trackers (announces);
    Handles_ [CurrentTorrent_].Handle_.force_reannounce ();
}

void Core::SetTrackers (int row, const QStringList& trackers)
{
	int old = CurrentTorrent_;
	CurrentTorrent_ = row;
	SetTrackers (trackers);
	CurrentTorrent_ = old;
}

QString Core::GetTorrentDirectory () const
{
    if (!CheckValidity (CurrentTorrent_))
        return QString ();

    return QString::fromUtf8 (Handles_.at (CurrentTorrent_).Handle_.save_path ().string ().c_str ());
}

bool Core::MoveTorrentFiles (const QString& newDir)
{
    if (!CheckValidity (CurrentTorrent_) || newDir == GetTorrentDirectory ())
        return false;

    Handles_.at (CurrentTorrent_).Handle_.move_storage (newDir.toUtf8 ().constData ());
    return true;
}

void Core::SetCurrentTorrent (int torrent)
{
    CurrentTorrent_ = torrent;
}

int Core::GetCurrentTorrent () const
{
	return CurrentTorrent_;
}

namespace
{
	bool FileFilter (const boost::filesystem::path& filename)
	{
		if (filename.leaf () [0] == '.')
			return false;
		return true;
	}

	void UpdateProgress (int i, int num, const QProgressDialog *pd)
	{
		qDebug () << Q_FUNC_INFO << i << num;
	}
}

void Core::MakeTorrent (NewTorrentParams params) const
{
    boost::filesystem::path::default_name_check (boost::filesystem::no_check);

	libtorrent::file_storage fs;
	libtorrent::file_pool fp;
	libtorrent::create_torrent ct (fs, params.PieceSize_);

	ct.set_creator ("LeechCraft BitTorrent");
	if (!params.Comment_.isEmpty ())
		ct.set_comment (params.Comment_.toUtf8 ());
	for (int i = 0; i < params.URLSeeds_.size (); ++i)
		ct.add_url_seed (params.URLSeeds_.at (0).toStdString ());
    ct.set_priv (!params.DHTEnabled_);

	if (params.DHTEnabled_)
		for (int i = 0; i < params.DHTNodes_.size (); ++i)
		{
			QStringList splitted = params.DHTNodes_.at (i).split (":");
			ct.add_node (std::pair<std::string, int> (splitted [0].trimmed ().toStdString (), splitted [1].trimmed ().toInt ()));
		}

	ct.add_tracker (params.AnnounceURL_.toStdString ());

	std::auto_ptr<QProgressDialog> pd (new QProgressDialog ());

	boost::filesystem::path fullPath =
		boost::filesystem::complete (params.Path_.toUtf8 ().constData ());
	libtorrent::add_files (fs, fullPath, FileFilter);
	libtorrent::set_piece_hashes (ct, fullPath.branch_path (),
			boost::bind (&UpdateProgress, _1, ct.num_pieces (), pd.get ()));

	libtorrent::entry e = ct.generate ();
	std::deque<char> outbuf;
	libtorrent::bencode (std::back_inserter (outbuf), e);

	QString filename = params.OutputDirectory_;
	if (!filename.endsWith ("/"))
		filename.append ("/");
	filename.append (params.TorrentName_);
	filename.append (".torrent");
	QFile file (filename);
	if (!file.open (QIODevice::WriteOnly | QIODevice::Truncate))
	{
		emit error (tr ("Could not open file %1 for write!").arg (filename));
		return;
	}
	for (size_t i = 0; i < outbuf.size (); ++i)
		file.write (&outbuf.at (i), 1);
	file.close ();
}

void Core::LogMessage (const QString& message)
{
    emit logMessage (message);
}

void Core::ImportData (const QByteArray& data)
{
}

QByteArray Core::ExportData () const
{
}

QString Core::GetStringForState (libtorrent::torrent_status::state_t state) const
{
    switch (state)
    {
        case libtorrent::torrent_status::queued_for_checking:
            return tr ("Queued for checking");
        case libtorrent::torrent_status::checking_files:
            return tr ("Checking files");
        case libtorrent::torrent_status::downloading_metadata:
            return tr ("Downloading metadata");
        case libtorrent::torrent_status::downloading:
            return tr ("Downloading");
        case libtorrent::torrent_status::finished:
            return tr ("Finished");
        case libtorrent::torrent_status::seeding:
            return tr ("Seeding");
        case libtorrent::torrent_status::allocating:
            return tr ("Allocating");
    }
    return "Uninitialized?!";
}

void Core::ReadSettings ()
{
}

void Core::RestoreTorrents ()
{
    QSettings settings (Proxy::Instance ()->GetOrganizationName (),
			Proxy::Instance ()->GetApplicationName () + "_Torrent");
    settings.beginGroup ("Core");
    int torrents = settings.beginReadArray ("AddedTorrents");
    for (int i = 0; i < torrents; ++i)
    {
        settings.setArrayIndex (i);
        boost::filesystem::path path = settings.value ("SavePath").toString ().toStdString ();
        QString filename = settings.value ("Filename").toString ();
        QFile torrent (QDir::homePath () + "/.leechcraft_bittorrent/" + filename);
        if (!torrent.open (QIODevice::ReadOnly))
        {
            emit error (tr ("Could not open saved torrent %1 for read.").arg (filename));
            continue;
        }
        QByteArray data = torrent.readAll ();
        torrent.close ();
        if (data.isEmpty ())
            continue;
        QByteArray resumed;

        libtorrent::torrent_handle handle = RestoreSingleTorrent (data, resumed, path);
        if (!handle.is_valid ())
            continue;

        std::vector<int> priorities;
        priorities.resize (settings.beginReadArray ("Priorities"));
        for (size_t j = 0; j < priorities.size (); ++j)
        {
            settings.setArrayIndex (j);
            priorities [j] = settings.value ("Priority", 1).toInt ();
        }
        settings.endArray ();

        if (!priorities.size ())
        {
            priorities.resize (handle.get_torrent_info ().num_files ());
            std::fill (priorities.begin (), priorities.end (), 1);
        }

        quint64 ub = settings.value ("UploadedBytes").value<quint64> ();

        handle.prioritize_files (priorities);
        QStringList trackers = settings.value ("TrackersOverride").toStringList ();
        if (!trackers.isEmpty ())
        {
            std::vector<libtorrent::announce_entry> announces;
            for (int i = 0; i < trackers.size (); ++i)
                announces.push_back (libtorrent::announce_entry (trackers.at (i).toStdString ()));
            handle.replace_trackers (announces);
        }

        TorrentStruct tmp = { ub, priorities, handle, data, filename };
        tmp.Tags_ = settings.value ("Tags").toStringList ();
		tmp.ID_ = settings.value ("ID").toInt ();
		IDPool_.erase (std::find (IDPool_.begin (), IDPool_.end (), tmp.ID_));
		tmp.Parameters_ = static_cast<LeechCraft::TaskParameters> (settings.value ("Parameters").toInt ());
        beginInsertRows (QModelIndex (), Handles_.size (), Handles_.size ());
        Handles_.append (tmp);
        endInsertRows ();
    }
    settings.endArray ();
    settings.endGroup ();
}

libtorrent::torrent_handle Core::RestoreSingleTorrent (const QByteArray& data, const QByteArray& resumeData, const boost::filesystem::path& path)
{
    libtorrent::entry e, resume;

    QVector<char> byteData (data.size ())
        , byteResumeData (resumeData.size ());

    for (int i = 0; i < data.size (); ++i)
        byteData [i] = data.at (i);
    for (int i = 0; i < resumeData.size (); ++i)
        byteResumeData [i] = resumeData [i];

    try
    {
        resume = libtorrent::bdecode (byteData.constBegin (), byteData.constEnd ());
    }
    catch (const libtorrent::invalid_encoding& e)
    {
        qWarning () << Q_FUNC_INFO << "bad resume data";
    }

    try
    {
        e = libtorrent::bdecode (byteData.constBegin (), byteData.constEnd ());
    }
    catch (const libtorrent::invalid_encoding& e)
    {
        emit error (tr ("Bad bencoding in saved torrent data"));
    }

    libtorrent::torrent_handle handle;
    try
    {
		handle = Session_->add_torrent (boost::intrusive_ptr<libtorrent::torrent_info> (new libtorrent::torrent_info (e)),
				path,
				libtorrent::entry (),
				libtorrent::storage_mode_allocate);
    }
    catch (const libtorrent::invalid_torrent_file& e)
    {
        emit error (tr ("Invalid saved torrent data"));
    }
    catch (const libtorrent::duplicate_torrent& e)
    {
        emit error (tr ("The just restored torrent already exists in the session, that's strange."));
    }

    return handle;
}

void Core::HandleSingleFinished (int i)
{
	TorrentStruct torrent = Handles_.at (i);
	libtorrent::torrent_info info = Handles_.at (i).Handle_
		.get_torrent_info ();
	QString where = QString::fromUtf8 (Handles_.at (i).Handle_
			.save_path ().string ().c_str ());

    QString name = QString::fromStdString (info.name ());
    QString string = tr ("Torrent finished: %1").arg (name);
    emit torrentFinished (string);

    for (libtorrent::torrent_info::file_iterator i = info.begin_files (),
			end = info.end_files (); i != end; ++i)
        emit fileFinished (QString::fromUtf8 (i->path.string ().c_str ()));

	if (!(torrent.Parameters_ & LeechCraft::DoNotSaveInHistory))
		emit addToHistory (QString::fromStdString (info.name ()),
				where,
				info.total_size (),
				QDateTime::currentDateTime ());

	emit taskFinished (torrent.ID_);
}

int Core::GetCurrentlyDownloading () const
{
    int result = 0;
    for (int i = 0; i < Handles_.size (); ++i)
        result += (Handles_.at (i).State_ == TSDownloading);
    return result;
}

int Core::GetCurrentlySeeding () const
{
    int result = 0;
    for (int i = 0; i < Handles_.size (); ++i)
        result += (Handles_.at (i).State_ == TSSeeding);
    return result;
}

void Core::ManipulateSettings ()
{
    SetOverallDownloadRate (XmlSettingsManager::Instance ()->
			Property ("DownloadRateLimit", 5000).toInt ());
    SetOverallUploadRate (XmlSettingsManager::Instance ()->
			Property ("UploadRateLimit", 5000).toInt ());
    SetMaxDownloadingTorrents (XmlSettingsManager::Instance ()->
			Property ("MaxDownloadingTorrents", 0).toInt ());
    SetMaxUploadingTorrents (XmlSettingsManager::Instance ()->
			Property ("MaxUploadingTorrents", 0).toInt ());
    SetDesiredRating (XmlSettingsManager::Instance ()->
			Property ("DesiredRating", 0).toInt ());

    XmlSettingsManager::Instance ()->RegisterObject ("TCPPortRange",
			this, "tcpPortRangeChanged");
    XmlSettingsManager::Instance ()->RegisterObject ("DHTEnabled",
			this, "dhtStateChanged");
    XmlSettingsManager::Instance ()->RegisterObject ("AutosaveInterval",
			this, "autosaveIntervalChanged");
    XmlSettingsManager::Instance ()->RegisterObject ("MaxUploads",
			this, "maxUploadsChanged");
    XmlSettingsManager::Instance ()->RegisterObject ("MaxConnections",
			this, "maxConnectionsChanged");
    XmlSettingsManager::Instance ()->RegisterObject ("TrackerProxyEnabled",
			this, "setProxySettings");
    XmlSettingsManager::Instance ()->RegisterObject ("TrackerProxyHost",
			this, "setProxySettings");
    XmlSettingsManager::Instance ()->RegisterObject ("TrackerProxyPort",
			this, "setProxySettings");
    XmlSettingsManager::Instance ()->RegisterObject ("TrackerProxyAuth",
			this, "setProxySettings");
    XmlSettingsManager::Instance ()->RegisterObject ("PeerProxyEnabled",
			this, "setProxySettings");
    XmlSettingsManager::Instance ()->RegisterObject ("PeerProxyHost",
			this, "setProxySettings");
    XmlSettingsManager::Instance ()->RegisterObject ("PeerProxyPort",
			this, "setProxySettings");
    XmlSettingsManager::Instance ()->RegisterObject ("PeerProxyAuth",
			this, "setProxySettings");

    XmlSettingsManager::Instance ()->RegisterObject ("UserAgent",
			this, "setGeneralSettings");
    XmlSettingsManager::Instance ()->RegisterObject ("TrackerCompletionTimeout",
			this, "setGeneralSettings");
    XmlSettingsManager::Instance ()->RegisterObject ("TrackerReceiveTimeout",
			this, "setGeneralSettings");
    XmlSettingsManager::Instance ()->RegisterObject ("StopTrackerTimeout",
			this, "setGeneralSettings");
    XmlSettingsManager::Instance ()->RegisterObject ("TrackerMaximumResponseLength",
			this, "setGeneralSettings");
    XmlSettingsManager::Instance ()->RegisterObject ("PieceTimeout",
			this, "setGeneralSettings");
    XmlSettingsManager::Instance ()->RegisterObject ("RequestQueueTime",
			this, "setGeneralSettings");
    XmlSettingsManager::Instance ()->RegisterObject ("MaxAllowedInRequestQueue",
			this, "setGeneralSettings");
    XmlSettingsManager::Instance ()->RegisterObject ("MaxOutRequestQueue",
			this, "setGeneralSettings");
    XmlSettingsManager::Instance ()->RegisterObject ("WholePiecesThreshold",
			this, "setGeneralSettings");
    XmlSettingsManager::Instance ()->RegisterObject ("PeerTimeout",
			this, "setGeneralSettings");
    XmlSettingsManager::Instance ()->RegisterObject ("UrlSeedTimeout",
			this, "setGeneralSettings");
    XmlSettingsManager::Instance ()->RegisterObject ("UrlSeedPipelineSize",
			this, "setGeneralSettings");
    XmlSettingsManager::Instance ()->RegisterObject ("UrlSeedWaitRetry",
			this, "setGeneralSettings");
    XmlSettingsManager::Instance ()->RegisterObject ("FilePoolSize",
			this, "setGeneralSettings");
    XmlSettingsManager::Instance ()->RegisterObject ("AllowMultipleConnectionsPerIP",
			this, "setGeneralSettings");
    XmlSettingsManager::Instance ()->RegisterObject ("MaxFailcount",
			this, "setGeneralSettings");
    XmlSettingsManager::Instance ()->RegisterObject ("MinReconnectTime",
			this, "setGeneralSettings");
    XmlSettingsManager::Instance ()->RegisterObject ("PeerConnectTimeout",
			this, "setGeneralSettings");
    XmlSettingsManager::Instance ()->RegisterObject ("IgnoreLimitsOnLocalNetwork",
			this, "setGeneralSettings");
    XmlSettingsManager::Instance ()->RegisterObject ("ConnectionSpeed",
			this, "setGeneralSettings");
    XmlSettingsManager::Instance ()->RegisterObject ("SendRedundantHave",
			this, "setGeneralSettings");
    XmlSettingsManager::Instance ()->RegisterObject ("LazyBitfields",
			this, "setGeneralSettings");
    XmlSettingsManager::Instance ()->RegisterObject ("InactivityTimeout",
			this, "setGeneralSettings");
    XmlSettingsManager::Instance ()->RegisterObject ("UnchokeInterval",
			this, "setGeneralSettings");
    XmlSettingsManager::Instance ()->RegisterObject ("OptimisticUnchokeMultiplier",
			this, "setGeneralSettings");
    XmlSettingsManager::Instance ()->RegisterObject ("AnnounceIP",
			this, "setGeneralSettings");
    XmlSettingsManager::Instance ()->RegisterObject ("NumWant",
			this, "setGeneralSettings");
    XmlSettingsManager::Instance ()->RegisterObject ("InitialPickerThreshold",
			this, "setGeneralSettings");
    XmlSettingsManager::Instance ()->RegisterObject ("AllowedFastSetSize",
			this, "setGeneralSettings");
    XmlSettingsManager::Instance ()->RegisterObject ("MaxOutstandingDiskBytesPerConnection",
			this, "setGeneralSettings");
    XmlSettingsManager::Instance ()->RegisterObject ("HandshakeTimeout",
			this, "setGeneralSettings");
    XmlSettingsManager::Instance ()->RegisterObject ("UseDHTAsFallback",
			this, "setGeneralSettings");
    XmlSettingsManager::Instance ()->RegisterObject ("FreeTorrentHashes",
			this, "setGeneralSettings");
    XmlSettingsManager::Instance ()->RegisterObject ("UPNPIgnoreNonrouters",
			this, "setGeneralSettings");
	XmlSettingsManager::Instance ()->RegisterObject ("SendBufferWatermark",
			this, "setGeneralSettings");
	XmlSettingsManager::Instance ()->RegisterObject ("AutoUploadSlots",
			this, "setGeneralSettings");
	XmlSettingsManager::Instance ()->RegisterObject ("UseParoleMode",
			this, "setGeneralSettings");
	XmlSettingsManager::Instance ()->RegisterObject ("CacheSize",
			this, "setGeneralSettings");
	XmlSettingsManager::Instance ()->RegisterObject ("CacheExpiry",
			this, "setGeneralSettings");
	XmlSettingsManager::Instance ()->RegisterObject ("OutgoingPorts",
			this, "setGeneralSettings");
	XmlSettingsManager::Instance ()->RegisterObject ("PeerTOS",
			this, "setGeneralSettings");
	XmlSettingsManager::Instance ()->RegisterObject ("DontCountSlowTorrents",
			this, "setGeneralSettings");
	XmlSettingsManager::Instance ()->RegisterObject ("AutoManageInterval",
			this, "setGeneralSettings");
	XmlSettingsManager::Instance ()->RegisterObject ("ShareRatioLimit",
			this, "setGeneralSettings");
	XmlSettingsManager::Instance ()->RegisterObject ("SeedTimeRatioLimit",
			this, "setGeneralSettings");
	XmlSettingsManager::Instance ()->RegisterObject ("SeedTimeLimit",
			this, "setGeneralSettings");
	XmlSettingsManager::Instance ()->RegisterObject ("CloseRedundantConnections",
			this, "setGeneralSettings");
	XmlSettingsManager::Instance ()->RegisterObject ("AutoScrapeInterval",
			this, "setGeneralSettings");
	XmlSettingsManager::Instance ()->RegisterObject ("AutoScrapeMinInterval",
			this, "setGeneralSettings");
	XmlSettingsManager::Instance ()->RegisterObject ("MaxPeerListSize",
			this, "setGeneralSettings");
	XmlSettingsManager::Instance ()->RegisterObject ("MinAnnounceInterval",
			this, "setGeneralSettings");

    XmlSettingsManager::Instance ()->RegisterObject ("MaxPeersReply",
			this, "setDHTSettings");
    XmlSettingsManager::Instance ()->RegisterObject ("SearchBranching",
			this, "setDHTSettings");
    XmlSettingsManager::Instance ()->RegisterObject ("ServicePort",
			this, "setDHTSettings");
    XmlSettingsManager::Instance ()->RegisterObject ("MaxDHTFailcount",
			this, "setDHTSettings");

	XmlSettingsManager::Instance ()->RegisterObject ("ScrapeInterval",
			this, "setScrapeInterval");
	XmlSettingsManager::Instance ()->RegisterObject ("ScrapeEnabled",
			this, "setScrapeInterval");

    TagsCompletionModel_->UpdateTags (XmlSettingsManager::Instance ()->Property ("TorrentTags", QStringList (tr ("untagged"))).toStringList ());
    RestoreTorrents ();
}

void Core::CheckDownloadQueue ()
{
	int mdt = XmlSettingsManager::Instance ()->property ("MaxDownloadingTorrents").toInt ();
	while (GetCurrentlyDownloading () < mdt)
	{
		HandleDict_t::iterator i = Handles_.begin ();
		for (HandleDict_t::iterator end = Handles_.end ();
				i != end; ++i)
			if (i->State_ == TSWaiting2Download)
				break;

		if (i == Handles_.end ())
			break;
		else
			ResumeTorrent (std::distance (Handles_.begin (), i));
	}
}

void Core::CheckUploadQueue ()
{
	int mdt = XmlSettingsManager::Instance ()->property ("MaxDownloadingTorrents").toInt ();
	while (GetCurrentlySeeding () < mdt)
	{
		HandleDict_t::iterator i = Handles_.begin ();
		for (HandleDict_t::iterator end = Handles_.end ();
				i != end; ++i)
			if (i->State_ == TSWaiting2Seed)
				break;

		if (i == Handles_.end ())
			break;
		else
			ResumeTorrent (std::distance (Handles_.begin (), i));
	}
}

QStringList Core::GetTagsForIndexImpl (int torrent) const
{
	if (!CheckValidity (torrent))
		return QStringList ();

	return Handles_.at (torrent).Tags_;
}

void Core::UpdateTagsImpl (const QStringList& tags, int torrent)
{
    if (!CheckValidity (torrent))
        return;

    Handles_ [torrent].Tags_ = tags;
    TagsCompletionModel_->UpdateTags (tags);
}

void Core::writeSettings ()
{
    QDir home = QDir::home ();
    if (!home.exists (".leechcraft_bittorrent"))
        if (!home.mkdir (".leechcraft_bittorrent"))
        {
            emit error (tr ("Could not create path %1/.leechcraft_bittorrent").arg (QDir::homePath ()));
            return;
        }

    XmlSettingsManager::Instance ()->setProperty ("TorrentTags", TagsCompletionModel_->GetTags ());

    QSettings settings (Proxy::Instance ()->GetOrganizationName (), Proxy::Instance ()->GetApplicationName () + "_Torrent");
    settings.beginGroup ("Core");
    settings.beginWriteArray ("AddedTorrents");
    for (int i = 0; i < Handles_.size (); ++i)
    {
        settings.setArrayIndex (i);
        if (!CheckValidity (i))
            continue;
		int oldCurrent = CurrentTorrent_;
		CurrentTorrent_ = i;
        try
        {
            QFile file_info (QDir::homePath () + "/.leechcraft_bittorrent/" + Handles_.at (i).TorrentFileName_);
            if (!file_info.open (QIODevice::WriteOnly))
            {
                emit error ("Cannot write settings! Cannot open files for write!");
                break;
            }
            file_info.write (Handles_.at (i).TorrentFileContents_);
            file_info.close ();

            settings.setValue ("SavePath", QString::fromStdString (Handles_.at (i).Handle_.save_path ().string ()));
            settings.setValue ("UploadedBytes", Handles_.at (i).UploadedBefore_ + Handles_.at (i).Handle_.status ().total_upload);
            settings.setValue ("Filename", Handles_.at (i).TorrentFileName_);
            settings.setValue ("TrackersOverride", GetTrackers ());
            settings.setValue ("Tags", Handles_.at (i).Tags_);
			settings.setValue ("ID", Handles_.at (i).ID_);
			settings.setValue ("Parameters", static_cast<int> (Handles_.at (i).Parameters_));

            settings.beginWriteArray ("Priorities");
            for (size_t j = 0; j < Handles_.at (i).FilePriorities_.size (); ++j)
            {
                settings.setArrayIndex (j);
                settings.setValue ("Priority", Handles_.at (i).FilePriorities_ [j]);
            }
            settings.endArray ();
        }
        catch (const std::exception& e)
        {
            qWarning () << Q_FUNC_INFO << e.what ();
        }
        catch (...)
        {
            qWarning () << Q_FUNC_INFO << "unknown exception";
        }
		CurrentTorrent_ = oldCurrent;
    }
    settings.endArray ();
    settings.endGroup ();
}

void Core::checkFinished ()
{
    for (int i = 0; i < Handles_.size (); ++i)
    {
        if (Handles_.at (i).State_ == TSSeeding)
            continue;

        libtorrent::torrent_status status = Handles_.at (i).Handle_.status ();
        libtorrent::torrent_status::state_t state = status.state;

        if (Handles_.at (i).State_ == TSWaiting2Download ||
            Handles_.at (i).State_ == TSWaiting2Seed)
            continue;

        if (status.paused)
        {
            Handles_ [i].State_ = TSIdle;
            continue;
        }

        switch (state)
        {
            case libtorrent::torrent_status::queued_for_checking:
            case libtorrent::torrent_status::checking_files:
            case libtorrent::torrent_status::allocating:
                Handles_ [i].State_ = TSPreparing;
                break;
            case libtorrent::torrent_status::downloading:
                if (Handles_ [i].State_ != TSDownloading)
                {
					int mdt = XmlSettingsManager::Instance ()->property ("MaxDownloadingTorrents").toInt ();
                    if (!mdt || GetCurrentlyDownloading () < mdt)
                        Handles_ [i].State_ = TSDownloading;
                    else
                    {
                        Handles_ [i].State_ = TSWaiting2Download;
                        Handles_ [i].Handle_.pause ();
                    }
                }
                break;
			case libtorrent::torrent_status::downloading_metadata:
				break;
            case libtorrent::torrent_status::finished:
            case libtorrent::torrent_status::seeding:
                TorrentState oldState = Handles_ [i].State_;
                Handles_ [i].State_ = TSSeeding;
                if (oldState == TSDownloading)
                {
					int mut = XmlSettingsManager::Instance ()->property ("MaxUploadingTorrents").toInt ();
					if (mut && GetCurrentlySeeding () >= mut)
					{
						Handles_.at (i).Handle_.pause ();
						Handles_ [i].State_ = TSWaiting2Seed;
					}

					CheckDownloadQueue ();

					HandleSingleFinished (i);
                }
                break;
        }
    }
}

struct BaseDispatcher
{
	virtual void Log (const QString& logstr) const
	{
		qWarning () << "<libtorrent> " << logstr;
		Core::Instance ()->LogMessage (QDateTime::currentDateTime ().toString () + " " + logstr);
	}
};

struct SimpleDispatcher : public BaseDispatcher
{
	void operator() (const libtorrent::listen_failed_alert& a) const
	{
		QString logstr = QString ("Failed to open any port for listening (%1).")
			.arg (QString::fromStdString (a.message ()));
		Log (logstr);
	}
	void operator() (const libtorrent::listen_succeeded_alert& a) const
	{
		QString logstr = QString ("Listen succeeded %1 (%2).")
			.arg (QString::fromStdString (a.endpoint.address ().to_string ()))
			.arg (QString::fromStdString (a.message ()));
		Log (logstr);
	}
	void operator() (const libtorrent::portmap_error_alert& a) const
	{
		QString logstr = QString ("NAT router was successfully"
				" found but some port mapping request failed (%1).")
			.arg (QString::fromStdString (a.message ()));
		Log (logstr);
	}
	void operator() (const libtorrent::portmap_alert& a) const
	{
		QString logstr = QString ("Port successfully mapped on a router (%1)")
			.arg (QString::fromStdString (a.message ()));
		Log (logstr);
	}
	void operator() (const libtorrent::file_error_alert& a) const
	{
		QString logstr = QString ("File IO failure (%1).").arg (QString::fromStdString (a.message ()));
		Log (logstr);
	}
	void operator() (const libtorrent::tracker_announce_alert& a) const
	{
		QString logstr = QString ("Tracker announce is sent (%1).").arg (QString::fromStdString (a.message ()));
		Log (logstr);
	}
	void operator() (const libtorrent::tracker_alert& a) const
	{
		QString logstr = QString ("Failed tracker request:"
			"%1.")
			.arg (QString::fromStdString (a.message ()));
		Log (logstr);
	}
	void operator() (const libtorrent::tracker_reply_alert& a) const
	{
		QString logstr = QString ("Tracker announce succeeded. %1 peers (%2).")
			.arg (a.num_peers)
			.arg (QString::fromStdString (a.message ()));
		Log (logstr);
	}
	void operator() (const libtorrent::tracker_warning_alert& a) const
	{
		QString logstr = QString ("Tracker announce has warning (%1).")
			.arg (QString::fromStdString (a.message ()));
		Log (logstr);
	}
	void operator() (const libtorrent::url_seed_alert& a) const
	{
		QString logstr = QString ("HTTP seed %1 name lookup failed (%2).")
			.arg (QString::fromStdString (a.url))
			.arg (QString::fromStdString (a.message ()));
		Log (logstr);
	}
	void operator() (const libtorrent::hash_failed_alert& a) const
	{
		QString logstr = QString ("Piece %1 hash check failed (%2).")
			.arg (a.piece_index)
			.arg (QString::fromStdString (a.message ()));
		Log (logstr);
	}
	void operator() (const libtorrent::peer_ban_alert& a) const
	{
		QString logstr = QString ("Peer %1 banned (%2).")
			.arg (QString::fromStdString (a.ip.address ().to_string ()))
			.arg (QString::fromStdString (a.message ()));
		Log (logstr);				
	}
	void operator() (const libtorrent::peer_error_alert& a) const
	{
		QString logstr = QString ("Peer %1 sent something bad (%2).")
			.arg (QString::fromStdString (a.ip.address ().to_string ()))
			.arg (QString::fromStdString (a.message ()));
		Log (logstr);
	}
	void operator() (const libtorrent::invalid_request_alert& a) const
	{
		QString logstr = QString ("Invalid incoming piece request by %1, "
			"piece %2, start %3, length %4 (%5).")
			.arg (QString ::fromStdString (a.ip.address ().to_string ()))
			.arg (a.request.piece)
			.arg (a.request.start)
			.arg (a.request.length)
			.arg (QString::fromStdString (a.message ()));
		Log (logstr);
	}
	void operator() (const libtorrent::torrent_finished_alert& a) const
	{
		QString logstr = QString ("Torrent finished (%1).")
			.arg (QString::fromStdString (a.message ()));
		Log (logstr);
	}
	void operator() (const libtorrent::metadata_failed_alert& a) const
	{
		QString logstr = QString ("Metadata hash failed (%1).")
			.arg (QString::fromStdString (a.message ()));
		Log (logstr);
	}
	void operator() (const libtorrent::metadata_received_alert& a) const
	{
		QString logstr = QString ("Metadata received (%1).")
			.arg (QString::fromStdString (a.message ()));
		Log (logstr);
	}
	void operator() (const libtorrent::fastresume_rejected_alert& a) const
	{
		QString logstr = QString ("Fast resume rejected (%1).")
			.arg (QString::fromStdString (a.message ()));
		Log (logstr);
	}
	void operator() (const libtorrent::peer_blocked_alert& a) const
	{
		QString logstr = QString ("Peer %1 blocked by the IP filter (%2).")
			.arg (QString::fromStdString (a.ip.to_string ()))
			.arg (QString::fromStdString (a.message ()));
		Log (logstr);
	}
	void operator() (const libtorrent::storage_moved_alert& a) const
	{
		QString logstr = QString ("Storage successfully moved for torrent %1.")
			.arg (QString::fromStdString (a.handle.name ()));
		Log (logstr);
	}
	void operator() (const libtorrent::torrent_paused_alert& a) const
	{
		QString logstr = QString ("Torrent %1 paused.")
			.arg (QString::fromStdString (a.handle.name ()));
		Log (logstr);
	}
	void operator() (const libtorrent::alert& a) const
	{
		QString logstr = QString ("General alert (%1).")
			.arg (QString::fromStdString (a.message ()));
		Log (logstr);
	}
};

void Core::queryLibtorrentForWarnings ()
{
	std::auto_ptr<libtorrent::alert> a;
	a = Session_->pop_alert ();
	SimpleDispatcher sd;
	while (a.get ())
	{
		try
		{
			libtorrent::handle_alert<
				libtorrent::listen_failed_alert
				, libtorrent::listen_succeeded_alert
				, libtorrent::portmap_error_alert
				, libtorrent::portmap_alert
				, libtorrent::file_error_alert
				, libtorrent::tracker_announce_alert
				, libtorrent::tracker_alert
				, libtorrent::tracker_reply_alert
				, libtorrent::tracker_warning_alert
				, libtorrent::url_seed_alert
				, libtorrent::hash_failed_alert
				, libtorrent::peer_ban_alert
				, libtorrent::peer_error_alert
				, libtorrent::invalid_request_alert
				, libtorrent::torrent_finished_alert
				, libtorrent::metadata_failed_alert
				, libtorrent::metadata_received_alert
				, libtorrent::fastresume_rejected_alert
				, libtorrent::peer_blocked_alert
				, libtorrent::storage_moved_alert
				, libtorrent::torrent_paused_alert
				, libtorrent::alert
				>::handle_alert (a, sd);
		}
		catch (const libtorrent::unhandled_alert& e)
		{
			qWarning () << Q_FUNC_INFO << "unhandled alert" << QString::fromStdString (a->message ());
		}
		a = Session_->pop_alert ();
	}
}

void Core::scrape ()
{
	for (HandleDict_t::iterator i = Handles_.begin (),
			end = Handles_.end (); i != end; ++i)
		i->Handle_.scrape_tracker ();
}

bool Core::CheckValidity (int pos) const
{
    if (pos >= Handles_.size () || pos < 0)
    {
        qWarning () << QString ("Torrent with position %1 doesn't exist in The List").arg (pos);
        return false;
    }
    if (!Handles_.at (pos).Handle_.is_valid ())
    {
        qWarning () << QString ("Torrent with position %1 found in The List, but is invalid").arg (pos);
        return false;
    }
    return true;
}

void Core::timerEvent (QTimerEvent *e)
{
    if (e->timerId () == InterfaceUpdateTimer_)
        emit dataChanged (index (0, 0), index (Handles_.size (), columnCount (QModelIndex ())));
}

void Core::tcpPortRangeChanged ()
{
    QList<QVariant> ports = XmlSettingsManager::Instance ()->property ("TCPPortRange").toList ();
    Session_->listen_on (std::make_pair (ports.at (0).toInt (), ports.at (1).toInt ()));
}

void Core::dhtStateChanged ()
{
    if (XmlSettingsManager::Instance ()->property ("DHTEnabled").toBool ())
        Session_->start_dht (libtorrent::entry ());
    else
        Session_->stop_dht ();
}

void Core::autosaveIntervalChanged ()
{
    SettingsSaveTimer_->stop ();
    SettingsSaveTimer_->start (XmlSettingsManager::Instance ()->property ("AutosaveInterval").toInt () * 1000);
}

void Core::maxUploadsChanged ()
{
    Session_->set_max_uploads (XmlSettingsManager::Instance ()->property ("MaxUploads").toInt ());
}

void Core::maxConnectionsChanged ()
{
    Session_->set_max_connections (XmlSettingsManager::Instance ()->property ("MaxConnections").toInt ());
}

void Core::setProxySettings ()
{
    libtorrent::proxy_settings trackerProxySettings, peerProxySettings;
    if (XmlSettingsManager::Instance ()->property ("TrackerProxyEnabled").toBool ())
    {
        trackerProxySettings.hostname = XmlSettingsManager::Instance ()->property ("TrackerProxyAddress").toString ().toStdString ();
        trackerProxySettings.port = XmlSettingsManager::Instance ()->property ("TrackerProxyPort").toInt ();
		QStringList auth = XmlSettingsManager::Instance ()->property ("TrackerProxyAuth").toString ().split ('@');
		if (auth.size ())
			trackerProxySettings.username = auth.at (0).toStdString ();
		if (auth.size () > 1)
			trackerProxySettings.password = auth.at (1).toStdString ();
        bool passworded = trackerProxySettings.username.size ();
        QString pt = XmlSettingsManager::Instance ()->property ("TrackerProxyType").toString ();
        if (pt == "http")
            trackerProxySettings.type = passworded ? libtorrent::proxy_settings::http_pw : libtorrent::proxy_settings::http;
        else if (pt == "socks4")
            trackerProxySettings.type = libtorrent::proxy_settings::socks4;
        else if (pt == "socks5")
            trackerProxySettings.type = passworded ? libtorrent::proxy_settings::socks5_pw : libtorrent::proxy_settings::socks5;
        else
            trackerProxySettings.type = libtorrent::proxy_settings::none;
    }
    else
        trackerProxySettings.type = libtorrent::proxy_settings::none;

    if (XmlSettingsManager::Instance ()->property ("PeerProxyEnabled").toBool ())
    {
        peerProxySettings.hostname = XmlSettingsManager::Instance ()->property ("PeerProxyAddress").toString ().toStdString ();
        peerProxySettings.port = XmlSettingsManager::Instance ()->property ("PeerProxyPort").toInt ();
		QStringList auth = XmlSettingsManager::Instance ()->property ("PeerProxyAuth").toString ().split ('@');
		if (auth.size ())
			peerProxySettings.username = auth.at (0).toStdString ();
		if (auth.size () > 1)
			peerProxySettings.password = auth.at (1).toStdString ();
        bool passworded = peerProxySettings.username.size ();
        QString pt = XmlSettingsManager::Instance ()->property ("PeerProxyType").toString ();
        if (pt == "http")
            peerProxySettings.type = passworded ? libtorrent::proxy_settings::http_pw : libtorrent::proxy_settings::http;
        else if (pt == "socks4")
            peerProxySettings.type = libtorrent::proxy_settings::socks4;
        else if (pt == "socks5")
            peerProxySettings.type = passworded ? libtorrent::proxy_settings::socks5_pw : libtorrent::proxy_settings::socks5;
        else
            peerProxySettings.type = libtorrent::proxy_settings::none;
    }
    else
        peerProxySettings.type = libtorrent::proxy_settings::none;

    Session_->set_peer_proxy (peerProxySettings);
    Session_->set_web_seed_proxy (peerProxySettings);
    Session_->set_tracker_proxy (trackerProxySettings);
}

void Core::setGeneralSettings ()
{
    libtorrent::session_settings settings = Session_->settings ();

    settings.user_agent = XmlSettingsManager::Instance ()->
		property ("UserAgent").toString ().toStdString ();
    settings.tracker_completion_timeout = XmlSettingsManager::Instance ()->
		property ("TrackerCompletionTimeout").toInt ();
    settings.tracker_receive_timeout = XmlSettingsManager::Instance ()->
		property ("TrackerReceiveTimeout").toInt ();
    settings.stop_tracker_timeout = XmlSettingsManager::Instance ()->
		property ("StopTrackerTimeout").toInt ();
    settings.tracker_maximum_response_length = XmlSettingsManager::Instance ()->
		property ("TrackerMaximumResponseLength").toInt () * 1024;
    settings.piece_timeout = XmlSettingsManager::Instance ()->
		property ("PieceTimeout").toInt ();
    settings.request_queue_time = XmlSettingsManager::Instance ()->
		property ("RequestQueueTime").toInt ();
    settings.max_allowed_in_request_queue = XmlSettingsManager::Instance ()->
		property ("MaxAllowedInRequestQueue").toInt ();
    settings.max_out_request_queue = XmlSettingsManager::Instance ()->
		property ("MaxOutRequestQueue").toInt ();
    settings.whole_pieces_threshold = XmlSettingsManager::Instance ()->
		property ("WholePiecesThreshold").toInt ();
    settings.peer_timeout = XmlSettingsManager::Instance ()->
		property ("PeerTimeout").toInt ();
    settings.urlseed_timeout = XmlSettingsManager::Instance ()->
		property ("UrlSeedTimeout").toInt ();
    settings.urlseed_pipeline_size = XmlSettingsManager::Instance ()->
		property ("UrlSeedPipelineSize").toInt ();
    settings.urlseed_wait_retry = XmlSettingsManager::Instance ()->
		property ("UrlSeedWaitRetry").toInt ();
    settings.file_pool_size = XmlSettingsManager::Instance ()->
		property ("FilePoolSize").toInt ();
    settings.allow_multiple_connections_per_ip = XmlSettingsManager::Instance ()->
		property ("AllowMultipleConnectionsPerIP").toBool ();
    settings.max_failcount = XmlSettingsManager::Instance ()->
		property ("MaxFailcount").toInt ();
    settings.min_reconnect_time = XmlSettingsManager::Instance ()->
		property ("MinReconnectTime").toInt ();
    settings.peer_connect_timeout = XmlSettingsManager::Instance ()->
		property ("PeerConnectTimeout").toInt ();
    settings.ignore_limits_on_local_network = XmlSettingsManager::Instance ()->
		property ("IgnoreLimitsOnLocalNetwork").toBool ();
    settings.connection_speed = XmlSettingsManager::Instance ()->
		property ("ConnectionSpeed").toInt ();
    settings.send_redundant_have = XmlSettingsManager::Instance ()->
		property ("SendRedundantHave").toBool ();
    settings.lazy_bitfields = XmlSettingsManager::Instance ()->
		property ("LazyBitfields").toBool ();
    settings.inactivity_timeout = XmlSettingsManager::Instance ()->
		property ("InactivityTimeout").toInt ();
    settings.unchoke_interval = XmlSettingsManager::Instance ()->
		property ("UnchokeInterval").toInt ();
    settings.optimistic_unchoke_multiplier = XmlSettingsManager::Instance ()->
		property ("OptimisticUnchokeMultiplier").toInt ();
    try
    {
        if (XmlSettingsManager::Instance ()->property ("AnnounceIP").toString ().isEmpty ())
            settings.announce_ip = boost::asio::ip::address ();
        else
            settings.announce_ip = boost::asio::ip::address::from_string (XmlSettingsManager::Instance ()->property ("AnnounceIP").toString ().toStdString ());
    }
    catch (...)
    {
        error (tr ("Wrong announce address %1").arg (XmlSettingsManager::Instance ()->property ("AnnounceIP").toString ()));
    }
    settings.num_want = XmlSettingsManager::Instance ()->
		property ("NumWant").toInt ();
    settings.initial_picker_threshold = XmlSettingsManager::Instance ()->
		property ("InitialPickerThreshold").toInt ();
    settings.allowed_fast_set_size = XmlSettingsManager::Instance ()->
		property ("AllowedFastSetSize").toInt ();
    settings.max_outstanding_disk_bytes_per_connection = XmlSettingsManager::Instance ()->
		property ("MaxOutstandingDiskBytesPerConnection").toInt () * 1024;
    settings.handshake_timeout = XmlSettingsManager::Instance ()->
		property ("HandshakeTimeout").toInt ();
    settings.use_dht_as_fallback = XmlSettingsManager::Instance ()->
		property ("UseDHTAsFallback").toBool ();
    settings.free_torrent_hashes = XmlSettingsManager::Instance ()->
		property ("FreeTorrentHashes").toBool ();
    settings.upnp_ignore_nonrouters = XmlSettingsManager::Instance ()->
		property ("UPNPIgnoreNonrouters").toBool ();
	settings.send_buffer_watermark = XmlSettingsManager::Instance ()->
		property ("SendBufferWatermark").toInt () * 1024;
	settings.auto_upload_slots = XmlSettingsManager::Instance ()->
		property ("AutoUploadSlots").toBool ();
	settings.use_parole_mode = XmlSettingsManager::Instance ()->
		property ("UseParoleMode").toBool ();
	settings.cache_size = 1048576 / 16384 * XmlSettingsManager::Instance ()->
		property ("CacheSize").value<long int> ();
	settings.cache_expiry = XmlSettingsManager::Instance ()->
		property ("CacheExpiry").toInt ();
	QList<QVariant> ports = XmlSettingsManager::Instance ()->
		property ("OutgoingPorts").toList ();
	if (ports.size () == 2)
		settings.outgoing_ports = std::make_pair (ports.at (0).toInt (),
				ports.at (1).toInt ());
	settings.peer_tos = XmlSettingsManager::Instance ()->
		property ("PeerTOS").toInt ();
	settings.dont_count_slow_torrents = XmlSettingsManager::Instance ()->
		property ("DontCountSlowTorrents").toBool ();
	settings.auto_manage_interval = XmlSettingsManager::Instance ()->
		property ("AutoManageInterval").toInt ();
	settings.share_ratio_limit = XmlSettingsManager::Instance ()->
		property ("ShareRatioLimit").toDouble ();
	settings.seed_time_ratio_limit = XmlSettingsManager::Instance ()->
		property ("SeedTimeRatioLimit").toDouble ();
	settings.seed_time_limit = XmlSettingsManager::Instance ()->
		property ("SeedTimeLimit").toULongLong () * 60;
	settings.close_redundant_connections = XmlSettingsManager::Instance ()->
		property ("CloseRedundantConnections").toBool ();
	settings.auto_scrape_interval = XmlSettingsManager::Instance ()->
		property ("AutoScrapeInterval").toInt () * 60;
	settings.auto_scrape_min_interval = XmlSettingsManager::Instance ()->
		property ("AutoScrapeMinInterval").toInt ();
	settings.max_peerlist_size = XmlSettingsManager::Instance ()->
		property ("MaxPeerListSize").toInt ();
	settings.min_announce_interval = XmlSettingsManager::Instance ()->
		property ("MinAnnounceInterval").toInt ();

    Session_->set_settings (settings);
}

void Core::setDHTSettings ()
{
    libtorrent::dht_settings settings;

    settings.max_peers_reply = XmlSettingsManager::Instance ()->property ("MaxPeersReply").toInt ();
    settings.search_branching = XmlSettingsManager::Instance ()->property ("SearchBranching").toInt ();
    settings.service_port = XmlSettingsManager::Instance ()->property ("ServicePort").toInt ();
    settings.max_fail_count = XmlSettingsManager::Instance ()->property ("MaxDHTFailcount").toInt ();

    Session_->set_dht_settings (settings);
}

void Core::setScrapeInterval ()
{
	bool scrapeEnabled = XmlSettingsManager::Instance ()->property ("ScrapeEnabled").toBool ();
	if (scrapeEnabled)
	{
		ScrapeTimer_->stop ();
		ScrapeTimer_->start (XmlSettingsManager::Instance ()->
				property ("ScrapeInterval").toInt () * 1000);
	}
	else
		ScrapeTimer_->stop ();
}

