#ifndef PLUGINS_DEADLYRICS_LYRICWIKISEARCHER_H
#define PLUGINS_DEADLYRICS_LYRICWIKISEARCHER_H
#include "searcher.h"
#include <vector>
#include <boost/shared_ptr.hpp>

class QHttp;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace DeadLyrics
		{
			class LyricWikiSearcher : public Searcher
			{
				Q_OBJECT
			public:
				LyricWikiSearcher ();
				void Start (const QStringList&, QByteArray&);
				void Stop (const QByteArray&);
			private slots:
				void handleFinished ();
			};
		};
	};
};

#endif

