#include "findproxy.h"
#include <QUrl>
#include <QtDebug>
#include "searcher.h"
#include "core.h"

FindProxy::FindProxy (const LeechCraft::Request& request,
		QObject *parent)
: QAbstractItemModel (parent)
, Request_ (request)
{
	QStringList subs = Request_.String_.split (" - ", QString::SkipEmptyParts);
	if (subs.size () < 2)
		return;

	searchers_t searchers = Core::Instance ().GetSearchers (Request_.Category_);
	for (searchers_t::iterator i = searchers.begin (),
			end = searchers.end (); i != end; ++i)
	{
		connect (i->get (),
				SIGNAL (textFetched (const Lyrics&, const QByteArray&)),
				this,
				SLOT (handleTextFetched (const Lyrics&, const QByteArray&)));
		Hashes_.push_back ((*i)->Start (subs));
	}
}

FindProxy::~FindProxy ()
{
	size_t size = Hashes_.size ();
	if (size)
	{
		searchers_t searchers = Core::Instance ().GetSearchers (Request_.Category_);
		for (size_t i = 0; i < size; ++i)
			searchers [i]->Stop (Hashes_ [i]);
	}
}

QAbstractItemModel* FindProxy::GetModel ()
{
	return this;
}

int FindProxy::columnCount (const QModelIndex&) const
{
	return 3;
}

QVariant FindProxy::data (const QModelIndex& index, int role) const
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
		case 2:
			return QString ("%1 (%2)")
				.arg (QUrl (lyrics.URL_).host ())
				.arg (lyrics.URL_);
		default:
			return Request_.Category_;
	}
}

QModelIndex FindProxy::index (int row, int column,
		const QModelIndex& parent) const
{
	if (!hasIndex (row, column, parent))
		return QModelIndex ();
	
	return createIndex (row, column);
}

QModelIndex FindProxy::parent (const QModelIndex&) const
{
	return QModelIndex ();
}

int FindProxy::rowCount (const QModelIndex& index) const
{
	return index.isValid () ? 0 : Lyrics_.size ();
}

void FindProxy::handleTextFetched (const Lyrics& lyrics,
		const QByteArray& hash)
{
	if (std::find (Hashes_.begin (), Hashes_.end (), hash) == Hashes_.end ())
		return;

	beginInsertRows (QModelIndex (), Lyrics_.size (), Lyrics_.size ());
	Lyrics_.push_back (lyrics);
	endInsertRows ();
}

