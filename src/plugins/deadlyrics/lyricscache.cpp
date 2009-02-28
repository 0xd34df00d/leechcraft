#include "lyricscache.h"
#include <stdexcept>
#include <QFile>
#include <QtDebug>
#include <plugininterface/util.h>

LyricsCache::LyricsCache ()
{
	try
	{
		LeechCraft::Util::CreateIfNotExists ("deadlyrics/cache");
	}
	catch (const std::runtime_error& e)
	{
		qWarning () << Q_FUNC_INFO << e.what ();
		return;
	}

	Dir_ = QDir::homePath ();
	Dir_.cd (".leechcraft/deadlyrics/cache");
}

LyricsCache& LyricsCache::Instance ()
{
	static LyricsCache lc;
	return lc;
}

Lyrics LyricsCache::GetLyrics (const QByteArray& hash) const
{
	if (Dir_.exists (hash.toHex ()))
	{
		QFile file (Dir_.filePath (hash.toHex ()));
		if (file.open (QIODevice::ReadOnly))
		{
			QByteArray raw = file.readAll ();
			QDataStream in (raw);
			Lyrics lyrics;
			in >> lyrics;
			return lyrics;
		}
		else
		{
			qWarning () << Q_FUNC_INFO
				<< "could not open (read) file"
				<< file.fileName ();
			throw std::runtime_error ("Could not open file");
		}
	}
	else
		throw std::runtime_error ("No such lyrics");
}

void LyricsCache::SetLyrics (const QByteArray& hash, const Lyrics& lyrics)
{
	QFile file (Dir_.filePath (hash.toHex ()));
	if (!file.open (QIODevice::WriteOnly | QIODevice::Truncate))
	{
		qWarning () << Q_FUNC_INFO
			<< "could not open (write|truncate) file"
			<< file.fileName ();
		return;
	}
	QByteArray data;
	QDataStream out (&data, QIODevice::WriteOnly);
	out << lyrics;
	file.write (data);
}

