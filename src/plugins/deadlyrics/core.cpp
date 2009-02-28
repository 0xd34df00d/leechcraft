#include "core.h"
#include <algorithm>
#include <boost/bind.hpp>
#include <QCryptographicHash>
#include <QUrl>
#include <QtDebug>
#include "lyricwikisearcher.h"

Core::Core ()
{
	qRegisterMetaType<Lyrics> ("Lyrics");
	qRegisterMetaTypeStreamOperators<Lyrics> ("Lyrics");
	Searchers_.push_back (searcher_ptr (new LyricWikiSearcher));
}

Core& Core::Instance ()
{
	static Core core;
	return core;
}

void Core::Release ()
{
	Searchers_.clear ();
}

void Core::SetNetworkAccessManager (QNetworkAccessManager *manager)
{
	Manager_ = manager;
}

QNetworkAccessManager* Core::GetNetworkAccessManager () const
{
	return Manager_;
}

QStringList Core::GetCategories () const
{
	return QStringList (tr ("lyrics"));
}

searchers_t Core::GetSearchers (const QString& category) const
{
	if (category == tr ("lyrics"))
		return Searchers_;
	else
		return searchers_t ();
}

