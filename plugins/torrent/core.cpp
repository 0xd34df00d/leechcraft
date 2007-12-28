#include <QFile>
#include <QFileInfo>
#include <bencode.hpp>
#include <entry.hpp>
#include <extensions/metadata_transfer.hpp>
#include <extensions/ut_pex.hpp>
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

	Headers_ << tr ("Name") << tr ("Downloaded") << tr ("Size") << tr ("Progress") << tr ("State") << tr ("Seeds/peers") << tr ("Dspeed") << tr ("Uspeed") << tr ("Remaining");
}

void Core::Release ()
{
	Session_->stop_dht ();
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
	return QVariant ();
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

QVariant Core::headerData (int column, Qt::Orientation, int role) const
{
	if (role != Qt::DisplayRole)
		return QVariant ();

	else
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
	Handles_ [id] = handle;
	emit torrentAdded (id);
}

void Core::RemoveTorrent (TorrentID_t id)
{
	if (!Handles_.contains (id))
	{
		emit error (tr ("Torrent with ID %1 doesn't exist in The Map").arg (id));
		return;
	}
	if (!Handles_ [id].is_valid ())
	{
		emit error (tr ("Torrent with ID %1 found in The Map, but is invalid, gonna remove it").arg (id));
		Handles_.remove (id);
		return;
	}

	Session_->remove_torrent (Handles_ [id]);
	Handles_.remove (id);
	emit torrentRemoved (id);
}

