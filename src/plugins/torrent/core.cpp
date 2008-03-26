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
#include <memory>
#include <bencode.hpp>
#include <entry.hpp>
#include <extensions/metadata_transfer.hpp>
#include <extensions/ut_pex.hpp>
#include <file_pool.hpp>
#include <hasher.hpp>
#include <storage.hpp>
#include <file.hpp>
#include <alert_types.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>
#include <plugininterface/proxy.h>
#include "core.h"
#include "xmlsettingsmanager.h"

Q_GLOBAL_STATIC (Core, CoreInstance);

Core* Core::Instance ()
{
    return CoreInstance ();
}

Core::Core (QObject *parent)
: QAbstractItemModel (parent)
{
}

void Core::DoDelayedInit ()
{
    try
    {
        Session_ = new libtorrent::session (libtorrent::fingerprint ("LB", 0, 1, 0, 2));
        QList<QVariant> ports = XmlSettingsManager::Instance ()->property ("TCPPortRange").toList ();
        Session_->listen_on (std::make_pair (ports.at (0).toInt (), ports.at (1).toInt ()));
        Session_->add_extension (&libtorrent::create_metadata_plugin);
        Session_->add_extension (&libtorrent::create_ut_pex_plugin);
        if (XmlSettingsManager::Instance ()->property ("DHTEnabled").toBool ())
            Session_->start_dht (libtorrent::entry ());
        Session_->set_max_uploads (XmlSettingsManager::Instance ()->property ("MaxUploads").toInt ());
        Session_->set_max_connections (XmlSettingsManager::Instance ()->property ("MaxConnections").toInt ());
        Session_->set_severity_level (libtorrent::alert::info);
        setProxySettings ();
    }
    catch (const asio::system_error&)
    {
        qWarning () << "Seems like address is already in use.";
    }

    Headers_ << tr ("Name") << tr ("Downloaded") << tr ("Uploaded") << tr ("Rating") << tr ("Size") << tr ("Progress") << tr ("State") << tr ("Seeds/peers") << tr ("Drate") << tr ("Urate") << tr ("Remaining");

    ReadSettings ();
    InterfaceUpdateTimer_ = startTimer (1000);
    SettingsSaveTimer_ = new QTimer (this);
    connect (SettingsSaveTimer_, SIGNAL (timeout ()), this, SLOT (writeSettings ()));
    SettingsSaveTimer_->start (XmlSettingsManager::Instance ()->property ("AutosaveInterval").toInt () * 1000);

    QTimer *finished = new QTimer (this);
    connect (finished, SIGNAL (timeout ()), this, SLOT (checkFinished ()));
    finished->start (1000);

    QTimer *warningWatchdog = new QTimer (this);
    connect (warningWatchdog, SIGNAL (timeout ()), this, SLOT (queryLibtorrentForWarnings ()));
    warningWatchdog->start (100);

    SetOverallDownloadRate (XmlSettingsManager::Instance ()->Property ("DownloadRateLimit", 5000).toInt ());
    SetOverallUploadRate (XmlSettingsManager::Instance ()->Property ("UploadRateLimit", 5000).toInt ());
    SetDesiredRating (XmlSettingsManager::Instance ()->Property ("DesiredRating", 0).toInt ());

    XmlSettingsManager::Instance ()->RegisterObject ("TCPPortRange", this, "tcpPortRangeChanged");
    XmlSettingsManager::Instance ()->RegisterObject ("DHTEnabled", this, "dhtStateChanged");
    XmlSettingsManager::Instance ()->RegisterObject ("AutosaveInterval", this, "autosaveIntervalChanged");
    XmlSettingsManager::Instance ()->RegisterObject ("MaxUploads", this, "maxUploadsChanged");
    XmlSettingsManager::Instance ()->RegisterObject ("MaxConnections", this, "maxConnectionsChanged");
    XmlSettingsManager::Instance ()->RegisterObject ("TrackerProxyEnabled", this, "setProxySettings");
    XmlSettingsManager::Instance ()->RegisterObject ("TrackerProxyHost", this, "setProxySettings");
    XmlSettingsManager::Instance ()->RegisterObject ("TrackerProxyPort", this, "setProxySettings");
    XmlSettingsManager::Instance ()->RegisterObject ("TrackerProxyLogin", this, "setProxySettings");
    XmlSettingsManager::Instance ()->RegisterObject ("TrackerProxyPassword", this, "setProxySettings");
    XmlSettingsManager::Instance ()->RegisterObject ("PeerProxyEnabled", this, "setProxySettings");
    XmlSettingsManager::Instance ()->RegisterObject ("PeerProxyHost", this, "setProxySettings");
    XmlSettingsManager::Instance ()->RegisterObject ("PeerProxyPort", this, "setProxySettings");
    XmlSettingsManager::Instance ()->RegisterObject ("PeerProxyLogin", this, "setProxySettings");
    XmlSettingsManager::Instance ()->RegisterObject ("PeerProxyPassword", this, "setProxySettings");

    RestoreTorrents ();
}

void Core::Release ()
{
    writeSettings ();
    Session_->stop_dht ();
    killTimer (InterfaceUpdateTimer_);
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

    if (!CheckValidity (row))
        return QVariant ();

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
            return QString::fromStdString (h.name ());
        case ColumnDownloaded:
            return Proxy::Instance ()->MakePrettySize (status.total_done);
        case ColumnUploaded:
            return Proxy::Instance ()->MakePrettySize (status.total_payload_upload + Handles_.at (row).UploadedBefore_);
        case ColumnRating:
            return QString::number (static_cast<float> (status.total_payload_upload + Handles_.at (row).UploadedBefore_) / status.total_payload_download, 'g', 2);
        case ColumnSize:
            return Proxy::Instance ()->MakePrettySize (status.total_wanted);
        case ColumnProgress:
            return QString::number (status.progress * 100, 'g', 4) + ("%");
        case ColumnState:
            return status.paused ? tr ("Idle") : GetStringForState (status.state);
        case ColumnSP:
            return QString::number (status.num_seeds) + "/" + QString::number (status.num_peers);
        case ColumnDSpeed:
            return Proxy::Instance ()->MakePrettySize (status.download_payload_rate) + tr ("/s");
        case ColumnUSpeed:
            return Proxy::Instance ()->MakePrettySize (status.upload_payload_rate) + tr ("/s");
        case ColumnRemaining:
            return Proxy::Instance ()->MakeTimeFromLong ((info.total_size () - status.total_done) / status.download_payload_rate).toString ();
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
        return libtorrent::torrent_info ();
    }
    return GetTorrentInfo (file.readAll ());
}

libtorrent::torrent_info Core::GetTorrentInfo (const QByteArray& data)
{
    std::vector<char> buffer;
    buffer.resize (data.size ());
    for (int i = 0; i < data.size (); ++i)
        buffer [i] = data.at (i);

    try
    {
        libtorrent::entry e = libtorrent::bdecode (buffer.begin (), buffer.end ());
        libtorrent::torrent_info result (e);
        return result;
    }
    catch (const libtorrent::invalid_encoding& e)
    {
        emit error (tr ("Bad bencoding in torrent file"));
        return libtorrent::torrent_info ();
    }
    catch (const libtorrent::invalid_torrent_file& e)
    {
        emit error (tr ("Invalid torrent file"));
        return libtorrent::torrent_info ();
    }
}

bool Core::IsValidTorrent (const QByteArray& torrentData) const
{
    std::vector<char> buffer (torrentData.size ());
    qCopy (torrentData.begin (), torrentData.end (), buffer.begin ());

    try
    {
        libtorrent::entry e = libtorrent::bdecode (buffer.begin (), buffer.end ());
        libtorrent::torrent_info result (e);
    }
    catch (...)
    {
        return false;
    }
    return true;
}

TorrentInfo Core::GetTorrentStats (int row) const
{
    if (!CheckValidity (row))
        return TorrentInfo ();

    libtorrent::torrent_handle handle = Handles_.at (row).Handle_;
    libtorrent::torrent_status status = handle.status ();
    libtorrent::torrent_info info = handle.get_torrent_info ();

    TorrentInfo result;
    result.Tracker_ = QString::fromStdString (status.current_tracker);
    result.State_ = status.paused ? tr ("Idle") : GetStringForState (status.state);
    result.Downloaded_ = status.total_done;
    result.Uploaded_ = status.total_payload_upload + Handles_.at (row).UploadedBefore_;
    result.TotalSize_ = status.total_wanted;
    result.FailedSize_ = status.total_failed_bytes;
    result.DHTNodesCount_ = info.nodes ().size ();
    result.TotalPieces_ = info.num_pieces ();
    result.DownloadedPieces_ = status.num_pieces;
    result.DownloadRate_ = status.download_rate;
    result.UploadRate_ = status.upload_rate;
    result.ConnectedPeers_ = status.num_peers;
    result.ConnectedSeeds_ = status.num_seeds;
    result.PieceSize_ = info.piece_length ();
    result.Progress_ = status.progress;
    result.DistributedCopies_ = status.distributed_copies;
    result.NextAnnounce_ = QTime (status.next_announce.hours (),
                                  status.next_announce.minutes (),
                                  status.next_announce.seconds ());
    result.AnnounceInterval_ = QTime (status.announce_interval.hours (),
                                      status.announce_interval.minutes (),
                                      status.announce_interval.seconds ());
    result.Pieces_ = status.pieces;
    return result;
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
    result.NumDHTTorrents_ = status.dht_torrents;
    return result;
}

QList<FileInfo> Core::GetTorrentFiles (int row) const
{
    if (!CheckValidity (row))
        return QList<FileInfo> ();

    QList<FileInfo> result;
    libtorrent::torrent_handle handle = Handles_.at (row).Handle_;
    libtorrent::torrent_info info = handle.get_torrent_info ();
    std::vector<float> progresses;
    handle.file_progress (progresses);
    for (libtorrent::torrent_info::file_iterator i = info.begin_files (); i != info.end_files (); ++i)
    {
        FileInfo fi;
        fi.Name_ = QString::fromStdString (i->path.string ());
        fi.Size_ = i->size;
        fi.Priority_ = Handles_.at (row).FilePriorities_.at (i - info.begin_files ());
        fi.Progress_ = progresses.at (i - info.begin_files ());
        result << fi;
    }

    return result;
}

QList<PeerInfo> Core::GetPeers (int row) const
{
    if (!CheckValidity (row))
        return QList<PeerInfo> ();

    QList<PeerInfo> result;
    std::vector<libtorrent::peer_info> peerInfos;
    Handles_.at (row).Handle_.get_peer_info (peerInfos);

    for (int i = 0; i < peerInfos.size (); ++i)
    {
        libtorrent::peer_info pi = peerInfos [i];
        PeerInfo ppi;
        ppi.IP_ = QString::fromStdString (pi.ip.address ().to_string ());
        ppi.Seed_ = pi.seed;
        ppi.DSpeed_ = pi.down_speed;
        ppi.USpeed_ = pi.up_speed;
        ppi.Downloaded_ = pi.total_download;
        ppi.Uploaded_ = pi.total_upload;
        ppi.Client_ = QString::fromUtf8 (pi.client.c_str ());
        ppi.Pieces_ = pi.pieces;
        result << ppi;
    }

    return result;
}

void Core::AddFile (const QString& filename, const QString& path, const QVector<bool>& files)
{
    if (!QFileInfo (filename).exists () || !QFileInfo (filename).isReadable ())
    {
        emit error (tr ("File %1 doesn't exist or could not be read").arg (filename));
        return;
    }

    libtorrent::torrent_handle handle;
    try
    {
        handle = Session_->add_torrent (GetTorrentInfo (filename), boost::filesystem::path (path.toStdString ()), libtorrent::entry (), libtorrent::storage_mode_sparse);
    }
    catch (const libtorrent::duplicate_torrent& e)
    {
        emit error (tr ("The torrent %1 with save path %2 already exists in the session").arg (filename).arg (path));
        return;
    }
    catch (const libtorrent::invalid_encoding& e)
    {
        emit error (tr ("Bad bencoding in torrent file"));
        return;
    }
    catch (const libtorrent::invalid_torrent_file& e)
    {
        emit error (tr ("Invalid torrent file"));
        return;
    }
    catch (const std::runtime_error& e)
    {
        emit error (tr ("Runtime error"));
        return;
    }

    std::vector<int> priorities;
    priorities.resize (handle.get_torrent_info ().num_files ());
    for (int i = 0; i < priorities.size (); ++i)
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
    Handles_.append (tmp);
    endInsertRows ();
    handle.resolve_countries (true);
    QTimer::singleShot (3000, this, SLOT (writeSettings ()));
}

void Core::RemoveTorrent (int pos)
{
    if (!CheckValidity (pos))
        return;

    Session_->remove_torrent (Handles_.at (pos).Handle_);
    beginRemoveRows (QModelIndex (), pos, pos);
    Handles_.removeAt (pos);
    endRemoveRows ();
}

void Core::PauseTorrent (int pos)
{
    if (!CheckValidity (pos))
        return;

    Handles_.at (pos).Handle_.pause ();
}

void Core::ResumeTorrent (int pos)
{
    if (!CheckValidity (pos))
        return;

    Handles_.at (pos).Handle_.resume ();
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

double Core::GetDesiredRating () const
{
    return XmlSettingsManager::Instance ()->property ("DesiredRating").toInt ();
}

void Core::SetTorrentDownloadRate (int val, int torrent)
{
    if (CheckValidity (torrent))
        Handles_.at (torrent).Handle_.set_download_limit (val == 0 ? -1 : val * 1024);
}

void Core::SetTorrentUploadRate (int val, int torrent)
{
    if (CheckValidity (torrent))
        Handles_.at (torrent).Handle_.set_upload_limit (val == 0 ? -1 : val * 1024);
}

void Core::SetTorrentDesiredRating (double val, int torrent)
{
    if (CheckValidity (torrent))
    {
        Handles_.at (torrent).Handle_.set_ratio (val ? 1/val : 0);
        Handles_ [torrent].Ratio_ = val;
    }
}

int Core::GetTorrentDownloadRate (int torrent) const
{
    if (CheckValidity (torrent))
        return Handles_.at (torrent).Handle_.download_limit () / 1024;
    else
        return -1;
}

int Core::GetTorrentUploadRate (int torrent) const
{
    if (CheckValidity (torrent))
        return Handles_.at (torrent).Handle_.upload_limit () / 1024;
    else
        return -1;
}

double Core::GetTorrentDesiredRating (int torrent) const
{
    if (CheckValidity (torrent))
        return Handles_.at (torrent).Ratio_;
    else
        return -1;
}

void Core::SetFilePriority (int torrent, int file, int priority)
{
    if (!CheckValidity (torrent))
        return;

    if (priority > 7)
        priority = 7;
    else if (priority < 0)
        priority = 0;

    try
    {
        Handles_ [torrent].FilePriorities_.at (file) = priority;
        Handles_.at (torrent).Handle_.prioritize_files (Handles_.at (torrent).FilePriorities_);
    }
    catch (...)
    {
        qWarning () << Q_FUNC_INFO << QString ("index for torrent %1, file %2 is out of bounds").arg (torrent).arg (file);
    }
}

int Core::GetFilePriority (int torrent, int file) const
{
    if (!CheckValidity (torrent))
        return -1;

    return Handles_.at (torrent).FilePriorities_ [file];
}

namespace
{
    void AddFiles (libtorrent::torrent_info& t, const boost::filesystem::path& p, const boost::filesystem::path& l)
    {
        if (l.leaf () [0] == '.')
            return;

        boost::filesystem::path f (p / l);
        if (boost::filesystem::is_directory (f))
            for (boost::filesystem::directory_iterator i (f), end; i != end; ++i)
                AddFiles (t, p, l / i->leaf ());
        else
            t.add_file (l, boost::filesystem::file_size (f));
    }
}

void Core::MakeTorrent (NewTorrentParams params) const
{
    boost::intrusive_ptr<libtorrent::torrent_info> info (new libtorrent::torrent_info);
    info->set_creator ("Leechcraft BitTorrent");
    if (!params.Comment_.isEmpty ())
        info->set_comment (params.Comment_.toUtf8 ());
    for (int i = 0; i < params.URLSeeds_.size (); ++i)
        info->add_url_seed (params.URLSeeds_.at (0).toStdString ());
    info->set_priv (!params.DHTEnabled_);

    if (params.DHTEnabled_)
        for (int i = 0; i < params.DHTNodes_.size (); ++i)
        {
            QStringList splitted = params.DHTNodes_.at (i).split (":");
            info->add_node (std::pair<std::string, int> (splitted [0].trimmed ().toStdString (), splitted [1].trimmed ().toInt ()));
        }

    boost::filesystem::path::default_name_check (boost::filesystem::no_check);
    boost::filesystem::path fullPath = boost::filesystem::complete (params.Path_.toStdString ());
    AddFiles (*info, fullPath.branch_path (), fullPath.leaf ());
    info->set_piece_size (params.PieceSize_);

    libtorrent::file_pool fp;
    boost::scoped_ptr<libtorrent::storage_interface> st (libtorrent::default_storage_constructor (info, fullPath.branch_path (), fp));
    info->add_tracker (params.AnnounceURL_.toStdString ());
    std::vector<char> buf (params.PieceSize_);
    QProgressDialog pd (tr ("Hashing..."), tr ("Cancel"), 0, info->num_pieces ());
    for (int i = 0; i < info->num_pieces (); ++i)
    {
        st->read (&buf [0], i, 0, info->piece_size (i));
        libtorrent::hasher h (&buf [0], info->piece_size (i));
        info->set_hash (i, h.final ());
        pd.setValue (i + 1);
    }

    libtorrent::entry e = info->create_torrent ();
    std::vector<char> outbuf;
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
    for (int i = 0; i < outbuf.size (); ++i)
        file.write (&outbuf.at (i), 1);
    file.close ();
}

QString Core::GetStringForState (libtorrent::torrent_status::state_t state) const
{
    switch (state)
    {
        case libtorrent::torrent_status::queued_for_checking:
            return tr ("Queued for checking");
        case libtorrent::torrent_status::checking_files:
            return tr ("Checking files");
        case libtorrent::torrent_status::connecting_to_tracker:
            return tr ("Connecting");
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
    QSettings settings (Proxy::Instance ()->GetOrganizationName (), Proxy::Instance ()->GetApplicationName () + "_Torrent");
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
        QFile resumeData (QDir::homePath () + "/.leechcraft_bittorrent/" + filename + ".resume");
        QByteArray resumed;
        if (resumeData.open (QIODevice::ReadOnly))
            resumed = resumeData.readAll ();
        else
            qWarning () << Q_FUNC_INFO << "could not open resume data for torrent" << filename;

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

        beginInsertRows (QModelIndex (), Handles_.size (), Handles_.size ());
        TorrentStruct tmp = { ub, priorities, handle, data, filename };
        Handles_.append (tmp);
        endInsertRows ();
        handle.resolve_countries (true);
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
        handle = Session_->add_torrent (libtorrent::torrent_info (e), path);
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

void Core::HandleSingleFinished (const libtorrent::torrent_info& info)
{
    QString name = QString::fromStdString (info.name ());
    QString string = tr ("Torrent finished: %1").arg (name);
    emit torrentFinished (string);

    for (libtorrent::torrent_info::file_iterator i = info.begin_files (); i != info.end_files (); ++i)
        emit fileFinished (QString::fromStdString (i->path.string ()));
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

    QSettings settings (Proxy::Instance ()->GetOrganizationName (), Proxy::Instance ()->GetApplicationName () + "_Torrent");
    settings.beginGroup ("Core");
    settings.beginWriteArray ("AddedTorrents");
    for (int i = 0; i < Handles_.size (); ++i)
    {
        settings.setArrayIndex (i);
        if (!CheckValidity (i))
            continue;
        try
        {
            libtorrent::entry resume = Handles_.at (i).Handle_.write_resume_data ();
            QVector<char> resumeBuf;
            libtorrent::bencode (std::back_inserter (resumeBuf), resume);
            QFile file_info (QDir::homePath () + "/.leechcraft_bittorrent/" + Handles_.at (i).TorrentFileName_);
            if (!file_info.open (QIODevice::WriteOnly))
            {
                emit error ("Cannot write settings! Cannot open files for write!");
                break;
            }
            file_info.write (Handles_.at (i).TorrentFileContents_);
            file_info.close ();

            QFile file_resume (QDir::homePath () + "/.leechcraft_bittorrent/" + Handles_.at (i).TorrentFileName_ + ".resume");
            if (file_resume.open (QIODevice::WriteOnly))
            {
                QByteArray data;
                for (int i = 0; i < resumeBuf.size (); ++i)
                    data.append (resumeBuf.at (i));
                file_resume.write (data);
                file_resume.close ();
            }
            else
                qWarning () << Q_FUNC_INFO << "could not open the resume file for write";

            settings.setValue ("SavePath", QString::fromStdString (Handles_.at (i).Handle_.save_path ().string ()));
            settings.setValue ("UploadedBytes", Handles_.at (i).UploadedBefore_ + Handles_.at (i).Handle_.status ().total_upload);
            settings.setValue ("Filename", Handles_.at (i).TorrentFileName_);

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

        if (status.paused)
            Handles_ [i].State_ = TSIdle;

        switch (state)
        {
            case libtorrent::torrent_status::queued_for_checking:
            case libtorrent::torrent_status::checking_files:
            case libtorrent::torrent_status::allocating:
                Handles_ [i].State_ = TSPreparing;
                break;
            case libtorrent::torrent_status::connecting_to_tracker:
            case libtorrent::torrent_status::downloading:
                Handles_ [i].State_ = TSDownloading;
                break;
            case libtorrent::torrent_status::finished:
            case libtorrent::torrent_status::seeding:
                Handles_ [i].State_ = TSSeeding;
                HandleSingleFinished (Handles_.at (i).Handle_.get_torrent_info ());
                break;
        }
    }
}

void Core::queryLibtorrentForWarnings ()
{
    std::auto_ptr<libtorrent::alert> alert = Session_->pop_alert ();
    if (!alert.get ())
        return;

    QString logstr;

    libtorrent::tracker_alert *ta = dynamic_cast<libtorrent::tracker_alert*> (alert.get ());
    libtorrent::url_seed_alert *usa = dynamic_cast<libtorrent::url_seed_alert*> (alert.get ());
    libtorrent::hash_failed_alert *hfa = dynamic_cast<libtorrent::hash_failed_alert*> (alert.get ());
    libtorrent::peer_ban_alert *pba = dynamic_cast<libtorrent::peer_ban_alert*> (alert.get ());
    libtorrent::peer_error_alert *pea = dynamic_cast<libtorrent::peer_error_alert*> (alert.get ());
    libtorrent::invalid_request_alert *ira = dynamic_cast<libtorrent::invalid_request_alert*> (alert.get ());
    if (ta)
        logstr.append (QString ("failed tracker request: status code %1, for %2 times").arg (ta->status_code).arg (ta->times_in_row));
    else if (usa)
        logstr.append (QString ("problems with URL seed %1").arg (usa->url.c_str ()));
    else if (hfa)
        logstr.append (QString ("piece hash failed, PN: %1").arg (hfa->piece_index));
    else if (pba)
        logstr.append (QString ("peer banned: %1").arg (pba->ip.address ().to_string ().c_str ()));
    else if (pea)
        logstr.append (QString ("peer error: %1").arg (pea->ip.address ().to_string ().c_str ()));
    else if (ira)
        logstr.append (QString ("invalid request: %1, request { piece %2, start %3, length %4 }")
            .arg (ira->ip.address ().to_string ().c_str ())
            .arg (ira->request.piece)
            .arg (ira->request.start)
            .arg (ira->request.length));
    else
        logstr.append ("just common failure");

    logstr.append (QString ("\r\nRaw message: ") + alert->msg ().c_str ());

    qWarning () << "<libtorrent> " << logstr;
    emit logMessage (QDateTime::currentDateTime ().toString () + " " + logstr);
}

bool Core::CheckValidity (int pos) const
{
    if (pos >= Handles_.size () || pos < 0)
    {
        emit error (tr ("Torrent with position %1 doesn't exist in The List").arg (pos));
        return false;
    }
    if (!Handles_.at (pos).Handle_.is_valid ())
    {
        emit const_cast<Core*> (this)->error (tr ("Torrent with position %1 found in The List, but is invalid").arg (pos));
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
    libtorrent::session_settings settings;
    libtorrent::proxy_settings trackerProxySettings, peerProxySettings;
    if (XmlSettingsManager::Instance ()->property ("TrackerProxyEnabled").toBool ())
    {
        trackerProxySettings.hostname = XmlSettingsManager::Instance ()->property ("TrackerProxyAddress").toString ().toStdString ();
        trackerProxySettings.port = XmlSettingsManager::Instance ()->property ("TrackerProxyPort").toInt ();
        trackerProxySettings.username = XmlSettingsManager::Instance ()->property ("TrackerProxyLogin").toString ().toStdString ();
        trackerProxySettings.password = XmlSettingsManager::Instance ()->property ("TrackerProxyPassword").toString ().toStdString ();
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
        peerProxySettings.username = XmlSettingsManager::Instance ()->property ("PeerProxyLogin").toString ().toStdString ();
        peerProxySettings.password = XmlSettingsManager::Instance ()->property ("PeerProxyPassword").toString ().toStdString ();
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

