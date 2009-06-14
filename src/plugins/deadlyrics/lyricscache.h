#ifndef PLUGINS_DEADLYRICS_LYRICSCACHE_H
#define PLUGINS_DEADLYRICS_LYRICSCACHE_H
#include <QObject>
#include <QString>
#include <QDir>
#include "searcher.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace DeadLyrics
		{
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
		};
	};
};

#endif

