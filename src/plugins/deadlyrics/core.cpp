#include "core.h"
#include <algorithm>
#include <boost/bind.hpp>
#include <QCryptographicHash>
#include <QUrl>
#include <QtDebug>
#include "lyricwikisearcher.h"

Core::Core ()
{
	Searchers_.push_back (new LyricWikiSearcher);
	for (searchers_t::iterator i = Searchers_.begin (),
			end = Searchers_.end (); i != end; ++i)
		connect (*i,
				SIGNAL (textFetched (const Lyrics&)),
				this,
				SLOT (handleTextFetched (const Lyrics&)));
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

int Core::columnCount (const QModelIndex&) const
{
	return 3;
}

QVariant Core::data (const QModelIndex& index, int role) const
{
	if (!index.isValid () || role != Qt::DisplayRole)
		return QVariant ();

	Lyrics lyrics = Lyrics_ [index.row ()];
	switch (index.column ())
	{
		case 0:
			{
				QString result = lyrics.Author_;
				if (!lyrics.Album_.isEmpty ())
					result.append (" - ").append (lyrics.Album_);
				result.append (" - ").append (lyrics.Title_);
				return result;
			}
		case 1:
			return QUrl (lyrics.URL_).host ();
		case 2:
			return lyrics.URL_;
	}
}

QModelIndex Core::index (int row, int column,
		const QModelIndex& parent) const
{
	if (!hasIndex (row, column, parent))
		return QModelIndex ();
	
	return createIndex (row, column);
}

QModelIndex Core::parent (const QModelIndex&) const
{
	return QModelIndex ();
}

int Core::rowCount (const QModelIndex& index) const
{
	return index.isValid () ? Lyrics_.size () : 0;
}

void Core::SetNetworkAccessManager (QNetworkAccessManager *manager)
{
	Manager_ = manager;
}

QNetworkAccessManager* Core::GetNetworkAccessManager () const
{
	return Manager_;
}

QByteArray Core::Start (const LeechCraft::Request& request)
{
	QStringList subs = request.String_.split ('-', QString::SkipEmptyParts);
	while (subs.size () < 3)
		subs << QString ();

	std::for_each (Searchers_.begin (), Searchers_.end (),
			boost::bind (&Searcher::Start,
				_1, subs));

	return QCryptographicHash::hash (subs.join ("").toUtf8 (),
			QCryptographicHash::Md5);
}

void Core::Stop (const QByteArray& hash)
{
	std::for_each (Searchers_.begin (), Searchers_.end (),
			boost::bind (&Searcher::Stop,
				_1, hash));
}

void Core::Reset ()
{
	std::for_each (Searchers_.begin (), Searchers_.end (),
			boost::bind (&Searcher::Stop,
				_1, QByteArray ()));
	Lyrics_.clear ();
}

void Core::handleTextFetched (const Lyrics& lyrics)
{
	if (std::find (Lyrics_.begin (), Lyrics_.end (), lyrics) !=
			Lyrics_.end ())
		return;

	beginInsertRows (QModelIndex (), Lyrics_.size (), Lyrics_.size ());
	Lyrics_.push_back (lyrics);
	endInsertRows ();
}

