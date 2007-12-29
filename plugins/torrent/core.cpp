#include <QFile>
#include <QFileInfo>
#include <QTimerEvent>
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
, CurrentID_ (0)
{
	Session_ = new libtorrent::session (libtorrent::fingerprint ("LB", 0, 0, 0, 2));
	QPair<int, int> ports = SettingsManager::Instance ()->GetPortRange ();
	Session_->listen_on (std::make_pair (ports.first, ports.second));
	Session_->add_extension (&libtorrent::create_metadata_plugin);
	Session_->add_extension (&libtorrent::create_ut_pex_plugin);
	Session_->start_dht (libtorrent::entry ());

	Headers_ << tr ("Name") << tr ("Downloaded") << tr ("Uploaded") << tr ("Size") << tr ("Progress") << tr ("State") << tr ("Seeds/peers") << tr ("Dspeed") << tr ("Uspeed") << tr ("Remaining");
	InterfaceUpdateTimer_ = startTimer (1000);
}

void Core::Release ()
{
	Session_->stop_dht ();
	killTimer (InterfaceUpdateTimer_);
}

int Core::columnCount (const QModelIndex& index) const
{
	if (index.isValid ())
		return 0;
	else
		return Headers_.size ();
}

QVariant Core::data (const QModelIndex& index, int role) const
{
	if (!index.isValid ())
		return QVariant ();

	if (role != Qt::DisplayRole)
		return QVariant ();

	int row = index.row (),
		column = index.column ();

	libtorrent::torrent_handle h = Handles_.at (row).second;
	if (!h.is_valid ())
	{
		emit const_cast<Core*> (this)->error (tr ("%1: for row %2 torrent handle is invalid, ID is %3").arg (Q_FUNC_INFO).arg (row).arg (Handles_.at (row).first));
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
			return QString ("NI");
		case ColumnSize:
			return Proxy::Instance ()->MakePrettySize (info.total_size ());
		case ColumnProgress:
			return QString::number (status.progress * 100) + ("%");
		case ColumnState:
			if (status.paused)
				return tr ("Idle");
			else
				return GetStringForState (status.state);
		case ColumnSP:
			return QString::number (status.num_seeds) + "/" + QString::number (status.num_peers);
		case ColumnDSpeed:
			return Proxy::Instance ()->MakePrettySize (status.download_rate) + tr ("/s");
		case ColumnUSpeed:
			return Proxy::Instance ()->MakePrettySize (status.upload_rate) + tr ("/s");
		case ColumnRemaining:
			return Proxy::Instance ()->MakeTimeFromLong ((info.total_size () - status.total_done) / status.download_rate).toString ();
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
	return false;
}

QModelIndex Core::index (int row, int column, const QModelIndex& index) const
{
	if (index.isValid () || !hasIndex (row, column))
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

TorrentInfo Core::GetTorrentStats (int row)
{
	if (!CheckValidity (row))
		return TorrentInfo ();

	libtorrent::torrent_handle handle = Handles_.at (row).second;
	libtorrent::torrent_status status = handle.status ();
	libtorrent::torrent_info info = handle.get_torrent_info ();

	TorrentInfo result;
	result.Tracker_ = QString::fromStdString (status.current_tracker);
	result.State_ = GetStringForState (status.state);
	result.Downloaded_ = status.total_done;
	result.TotalSize_ = info.total_size ();
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
	return result;
}

void Core::AddFile (const QString& filename, const QString& path)
{
	if (!QFileInfo (filename).exists () || !QFileInfo (filename).isReadable ())
	{
		emit error (tr ("File %1 doesn't exist or could not be read").arg (filename));
		return;
	}

	libtorrent::torrent_handle handle;
	try
	{
		handle = Session_->add_torrent (GetTorrentInfo (filename), boost::filesystem::path (path.toStdString ()), libtorrent::entry (), false);
	}
	catch (const libtorrent::duplicate_torrent& e)
	{
		emit error (tr ("The torrent %1 with save path %2 already exists in the session").arg (filename).arg (path));
		return;
	}
	TorrentID_t id = CurrentID_++;
	beginInsertRows (QModelIndex (), Handles_.size (), Handles_.size ());
	Handles_.append (qMakePair (id, handle));
	endInsertRows ();
}

void Core::RemoveTorrent (int pos)
{
	if (!CheckValidity (pos))
		return;

	Session_->remove_torrent (Handles_.at (pos).second);
	Handles_.removeAt (pos);
}

void Core::PauseTorrent (int pos)
{
	if (!CheckValidity (pos))
		return;

	Handles_.at (pos).second.pause ();
}

void Core::ResumeTorrent (int pos)
{
	if (!CheckValidity (pos))
		return;

	Handles_.at (pos).second.resume ();
}

Core::HandleDict_t::iterator Core::FindTorrentByID (Core::TorrentID_t id)
{
	HandleDict_t::iterator i = Handles_.begin ();
	for ( ; i != Handles_.end (); ++i)
		if (i->first == id)
			break;

	return i;
}


Core::HandleDict_t::const_iterator Core::FindTorrentByID (Core::TorrentID_t id) const
{
	HandleDict_t::const_iterator i = Handles_.constBegin ();
	for ( ; i != Handles_.constEnd (); ++i)
		if (i->first == id)
			break;

	return i;
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
}

bool Core::CheckValidity (int pos)
{
	if (pos >= Handles_.size () || pos < 0)
	{
		emit error (tr ("Torrent with position %1 doesn't exist in The List").arg (pos));
		return false;
	}
	if (!Handles_.at (pos).second.is_valid ())
	{
		emit error (tr ("Torrent with position %1 found in The List, but is invalid").arg (pos));
		return false;
	}
	return true;
}

void Core::timerEvent (QTimerEvent *e)
{
	if (e->timerId () == InterfaceUpdateTimer_)
		emit dataChanged (index (0, 0), index (Handles_.size (), columnCount (QModelIndex ())));
}

