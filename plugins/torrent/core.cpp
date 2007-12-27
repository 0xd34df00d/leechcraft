#include <QFile>
#include <bencode.hpp>
#include <entry.hpp>
#include "core.h"

Q_GLOBAL_STATIC (Core, CoreInstance);

Core* Core::Instance ()
{
	return CoreInstance ();
}

Core::Core (QObject *parent)
: QObject (parent)
{
}

void Core::Release ()
{
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

	libtorrent::entry e = libtorrent::bdecode (buffer.begin (), buffer.end ());
	libtorrent::torrent_info result (e);
	return result;
}

