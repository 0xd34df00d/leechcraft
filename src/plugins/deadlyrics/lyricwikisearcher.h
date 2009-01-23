#ifndef LYRICWIKISEARCHER_H
#define LYRICWIKISEARCHER_H
#include "searcher.h"
#include <vector>
#include <boost/shared_ptr.hpp>

class QHttp;

class LyricWikiSearcher : public Searcher
{
	Q_OBJECT
public:
	LyricWikiSearcher ();
	void Start (const QString&, const QString&,
			const QString& = QString ());
	void Stop ();
private slots:
	void handleFinished ();
};

#endif

