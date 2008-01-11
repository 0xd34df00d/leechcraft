#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QTimerEvent>
#include <QSettings>
#include <QTimer>
#include <QtDebug>
#include <bencode.hpp>
#include <entry.hpp>
#include <extensions/metadata_transfer.hpp>
#include <extensions/ut_pex.hpp>
#include <plugininterface/proxy.h>
#include "core.h"
#include "settingsmanager.h"

Q_GLOBAL_STATIC (Core, CoreInstance);

Core* Core::Instance ()
{
	return CoreInstance ();
}

Core::Core (QObject *parent)
: QAbstractItemModel (parent)
{
	Session_ = new libtorrent::session (libtorrent::fingerprint ("LB", 0, 1, 0, 2));
	QPair<int, int> ports = SettingsManager::Instance ()->GetPortRange ();
	try
	{
		Session_->listen_on (std::make_pair (ports.first, ports.second));
		Session_->add_extension (&libtorrent::create_metadata_plugin);
		Session_->add_extension (&libtorrent::create_ut_pex_plugin);
		if (SettingsManager::Instance ()->GetDHTEnabled ())
			Session_->start_dht (libtorrent::entry ());
	}
	catch (const asio::system_error&)
	{
		qWarning () << "Seems like address is already in use.";
	}

	Headers_ << tr ("Name") << tr ("Downloaded") << tr ("Uploaded") << tr ("Size") << tr ("Progress") << tr ("State") << tr ("Seeds/peers") << tr ("Drate") << tr ("Urate") << tr ("Remaining");

	ReadSettings ();

	InterfaceUpdateTimer_ = startTimer (1000);

	SettingsSaveTimer_ = new QTimer (this);
	connect (SettingsSaveTimer_, SIGNAL (timeout ()), this, SLOT (writeSettings ()));
	SettingsSaveTimer_->start (SettingsManager::Instance ()->GetAutosaveInterval () * 1000);

	SettingsManager::Instance ()->RegisterObject ("DHTState", this, "dhtStateChanged");
}

void Core::DoDelayedInit ()
{
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
		case ColumnSize:
			return Proxy::Instance ()->MakePrettySize (status.total_wanted);
		case ColumnProgress:
			return QString::number (static_cast<float> (static_cast<int> (status.progress * 1000)) / 10) + ("%");
		case ColumnState:
			if (status.paused)
				return tr ("Idle");
			else
				return GetStringForState (status.state);
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

bool Core::hasChildren (const QModelIndex&) const
{
	return true;
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
	result.NextAnnounce_ = QTime (status.next_announce.hours (),
								  status.next_announce.minutes (),
								  status.next_announce.seconds ());
	result.AnnounceInterval_ = QTime (status.announce_interval.hours (),
									  status.announce_interval.minutes (),
									  status.announce_interval.seconds ());
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
		ppi.Country_ = QString::fromLocal8Bit (pi.country, 2);
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
		handle = Session_->add_torrent (GetTorrentInfo (filename), boost::filesystem::path (path.toStdString ()), libtorrent::entry (), libtorrent::storage_mode_allocate);
	}
	catch (const libtorrent::duplicate_torrent& e)
	{
		emit error (tr ("The torrent %1 with save path %2 already exists in the session").arg (filename).arg (path));
		return;
	}

	std::vector<int> priorities;
	priorities.resize (files.size ());
	for (int i = 0; i < files.size (); ++i)
		priorities [i] = files [i];

	handle.prioritize_files (priorities);

	beginInsertRows (QModelIndex (), Handles_.size (), Handles_.size ());
	TorrentStruct tmp = { 0, priorities, handle };
	Handles_.append (tmp);
	endInsertRows ();
	handle.resolve_countries (true);
	QTimer::singleShot (1000, this, SLOT (writeSettings ()));
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

void Core::MakeTorrent (NewTorrentParams params) const
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
	QSettings settings (Proxy::Instance ()->GetOrganizationName (), Proxy::Instance ()->GetApplicationName ());
	settings.beginGroup ("Torrent");
	settings.beginGroup ("Core");
	int torrents = settings.beginReadArray ("AddedTorrents");
	for (int i = 0; i < torrents; ++i)
	{
		settings.setArrayIndex (i);
		boost::filesystem::path path = settings.value ("SavePath").toString ().toStdString ();
		QPair<QVector<char>, QVector<char> > datas = ReadDataFor (i);
		if (datas.first.isEmpty ())
			continue;

		libtorrent::torrent_handle handle = RestoreSingleTorrent (datas.first, datas.second, path);
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
		TorrentStruct tmp = { ub, priorities, handle };
		Handles_.append (tmp);
		endInsertRows ();
		handle.resolve_countries (true);
	}
	settings.endArray ();
	settings.endGroup ();
	settings.endGroup ();
}

QPair<QVector<char>, QVector<char> > Core::ReadDataFor (int i) const
{
	QFile file_info		(QDir::homePath () + "/.leechcraft_bittorrent/" + QString ("%1_torrent_info.bncd").arg (i)),
		  file_resume	(QDir::homePath () + "/.leechcraft_bittorrent/" + QString ("%1_torrent_resume.bncd").arg (i));

	QVector<char> data, resumeData;
	if (!file_info.open (QIODevice::ReadOnly) || !file_resume.open (QIODevice::ReadOnly))
	{
		emit error (tr ("Could not open files to read settings :("));
		return qMakePair (data, resumeData);
	}

	for (int j = 0; j < file_info.size (); ++j)
	{
		char ch;
		file_info.read (&ch, 1);
		data.append (ch);
	}
	for (int j = 0; j < file_resume.size (); ++j)
	{
		char ch;
		file_resume.read (&ch, 1);
		resumeData.append (ch);
	}
	file_info.close ();
	file_resume.close ();

	return qMakePair (data, resumeData);
}

libtorrent::torrent_handle Core::RestoreSingleTorrent (const QVector<char>& data, const QVector<char>& resumeData, const boost::filesystem::path& path)
{
	libtorrent::entry e;
	libtorrent::entry resume;
	try
	{
		resume = libtorrent::bdecode (resumeData.constBegin (), resumeData.constEnd ());
	}
	catch (const libtorrent::invalid_encoding& e)
	{
	}

	try
	{
		e = libtorrent::bdecode (data.constBegin (), data.constEnd ());
	}
	catch (const libtorrent::invalid_encoding& e)
	{
		emit error (tr ("Bad bencoding in saved torrent data"));
	}

	libtorrent::torrent_handle handle;
	try
	{
		handle = Session_->add_torrent (libtorrent::torrent_info (e), path, resume, libtorrent::storage_mode_allocate);
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

void Core::writeSettings ()
{
	QDir home = QDir::home ();
	if (!home.exists (".leechcraft_bittorrent"))
		if (!home.mkdir (".leechcraft_bittorrent"))
		{
			emit error (tr ("Could not create path %1/.leechcraft_bittorrent").arg (QDir::homePath ()));
			return;
		}

	QSettings settings (Proxy::Instance ()->GetOrganizationName (), Proxy::Instance ()->GetApplicationName ());
	settings.beginGroup ("Torrent");
	settings.beginGroup ("Core");
	settings.beginWriteArray ("AddedTorrents");
	for (int i = 0; i < Handles_.size (); ++i)
	{
		settings.setArrayIndex (i);

		libtorrent::entry e = Handles_.at (i).Handle_.get_torrent_info ().create_torrent ();
		QVector<char> buf;
		libtorrent::bencode (std::back_inserter (buf), e);
		QFile file_info (QDir::homePath () + "/.leechcraft_bittorrent/" + QString ("%1_torrent_info.bncd").arg (i));
		if (!file_info.open (QIODevice::WriteOnly))
		{
			emit error ("Cannot write settings! Cannot open files for write!");
			break;
		}
		file_info.write (&buf.at (0), buf.size ());
		file_info.close ();

		buf.clear ();
		libtorrent::entry resume;
		try
		{
			resume = Handles_.at (i).Handle_.write_resume_data ();
		}
		catch (const libtorrent::invalid_handle&)
		{
		}
		libtorrent::bencode (std::back_inserter (buf), resume);
		QFile file_resume (QDir::homePath () + "/.leechcraft_bittorrent/" + QString ("%1_torrent_resume.bncd").arg (i));
		if (!file_resume.open (QIODevice::WriteOnly))
		{
			emit error ("Cannot write settings! Cannot open files for write!");
			break;
		}
		file_resume.write (&buf.at (0), buf.size ());
		file_resume.close ();

		settings.setValue ("SavePath", QString::fromStdString (Handles_.at (i).Handle_.save_path ().string ()));
		settings.setValue ("UploadedBytes", Handles_.at (i).UploadedBefore_ + Handles_.at (i).Handle_.status ().total_upload);

		settings.beginWriteArray ("Priorities");
		for (size_t j = 0; j < Handles_.at (i).FilePriorities_.size (); ++j)
		{
			settings.setArrayIndex (j);
			settings.setValue ("Priority", Handles_.at (i).FilePriorities_ [j]);
		}
		settings.endArray ();
	}
	settings.endArray ();
	settings.endGroup ();
	settings.endGroup ();
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

void Core::dhtStateChanged ()
{
	if (SettingsManager::Instance ()->GetDHTEnabled ())
		Session_->start_dht (libtorrent::entry ());
	else
		Session_->stop_dht ();
}

void Core::autosaveIntervalChanged ()
{
	SettingsSaveTimer_->stop ();
	SettingsSaveTimer_->start (SettingsManager::Instance ()->GetAutosaveInterval () * 1000);
}

