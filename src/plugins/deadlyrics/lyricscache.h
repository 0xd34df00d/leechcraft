#ifndef LYRICSCACHE_H
#define LYRICSCACHE_H
#include <QObject>
#include <QString>
#include <QDir>
#include "searcher.h"

class LyricsCache : public QObject
{
	Q_OBJECT
	
	QDir Dir_;
	LyricsCache ();
public:
	static LyricsCache& Instance ();
	Lyrics GetLyrics (const QByteArray&) const;
	void SetLyrics (const QByteArray&, const Lyrics&);
};

#endif

