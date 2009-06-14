#ifndef PLUGINS_DEADLYRICS_SEARCHER_H
#define PLUGINS_DEADLYRICS_SEARCHER_H
#include <vector>
#include <QObject>
#include <interfaces/ifinder.h>

class QDataStream;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace DeadLyrics
		{
			struct Lyrics
			{
				QString Author_;
				QString Album_;
				QString Title_;
				QString Text_;
				QString URL_;
			};

			bool operator== (const Lyrics&, const Lyrics&);
			QDataStream& operator<< (QDataStream&, const Lyrics&);
			QDataStream& operator>> (QDataStream&, Lyrics&);
			typedef std::vector<Lyrics> lyrics_t;

			class Searcher : public QObject
			{
				Q_OBJECT
			public:
				virtual ~Searcher ();
				virtual void Start (const QStringList&, QByteArray&) = 0;
				virtual void Stop (const QByteArray&) = 0;
			signals:
				void textFetched (const Lyrics&, const QByteArray&);
				void error (const QString&);
			};

			typedef boost::shared_ptr<Searcher> searcher_ptr;
			typedef std::vector<searcher_ptr> searchers_t;
		};
	};
};

#endif

