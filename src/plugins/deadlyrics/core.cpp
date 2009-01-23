#include "core.h"
#include <algorithm>
#include <boost/bind.hpp>
#include <QCryptographicHash>
#include <QtDebug>
#include "lyricwikisearcher.h"

Core::Core ()
{
	Searchers_.push_back (new LyricWikiSearcher);
	for (searchers_t::iterator i = Searchers_.begin (),
			end = Searchers_.end (); i != end; ++i)
		connect (*i,
				SIGNAL (textFetched (const QString&)),
				this,
				SLOT (handleTextFetched (const QString&)));
}

Core& Core::Instance ()
{
	static Core core;
	return core;
}

void Core::Release ()
{
	qDeleteAll (Searchers_);
}

void Core::SetNetworkAccessManager (QNetworkAccessManager *manager)
{
	Manager_ = manager;
}

QNetworkAccessManager* Core::GetNetworkAccessManager () const
{
	return Manager_;
}

void Core::Start (const QString& string)
{
	QStringList subs = string.split ('-', QString::SkipEmptyParts);
	while (subs.size () < 3)
		subs << QString ();

	std::for_each (Searchers_.begin (), Searchers_.end (),
			boost::bind (&Searcher::Start,
				_1,
				subs.at (0).trimmed (),
				subs.at (1).trimmed (),
				subs.at (2).trimmed ()));
}

void Core::Abort ()
{
	std::for_each (Searchers_.begin (), Searchers_.end (),
			boost::bind (&Searcher::Stop,
				_1));
}

void Core::handleTextFetched (const QString& text)
{
	LeechCraft::FoundEntity e;
	e.Categories_ = QStringList ("lyrics");
	e.Hashes_ [LeechCraft::FoundEntity::HTMD4] =
		QCryptographicHash::hash (text.toUtf8 (), QCryptographicHash::Md4);
	e.Hashes_ [LeechCraft::FoundEntity::HTMD5] =
		QCryptographicHash::hash (text.toUtf8 (), QCryptographicHash::Md5);
	e.Hashes_ [LeechCraft::FoundEntity::HTSHA1] =
		QCryptographicHash::hash (text.toUtf8 (), QCryptographicHash::Sha1);
	e.Description_ = text;
	emit entityUpdated (e);
}

