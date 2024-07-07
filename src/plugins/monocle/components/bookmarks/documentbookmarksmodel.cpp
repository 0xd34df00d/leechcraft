/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "documentbookmarksmodel.h"
#include <QtDebug>

namespace LC::Monocle
{
	DocumentBookmarksModel::DocumentBookmarksModel (QVector<Bookmark> bookmarks, QObject *parent)
	: QAbstractItemModel { parent }
	, Bookmarks_ { std::move (bookmarks) }
	{
	}

	QModelIndex DocumentBookmarksModel::index (int row, int column, const QModelIndex& parent) const
	{
		if (parent.isValid () || !hasIndex (row, column, parent))
			return {};

		return createIndex (row, column);
	}

	QModelIndex DocumentBookmarksModel::parent (const QModelIndex&) const
	{
		return {};
	}

	int DocumentBookmarksModel::rowCount (const QModelIndex& parent) const
	{
		return parent.isValid () ? 0 : Bookmarks_.size ();
	}

	int DocumentBookmarksModel::columnCount (const QModelIndex& parent) const
	{
		return parent.isValid () ? 0 : 1;
	}

	QVariant DocumentBookmarksModel::data (const QModelIndex& index, int role) const
	{
		switch (role)
		{
		case Qt::DisplayRole:
			return Bookmarks_ [index.row ()].Name_;
		default:
			return {};
		}
	}

	QVariant DocumentBookmarksModel::headerData (int section, Qt::Orientation orientation, int role) const
	{
		if (orientation != Qt::Horizontal || role != Qt::DisplayRole || section != 0)
			return {};

		return tr ("Name");
	}

	Bookmark DocumentBookmarksModel::GetBookmark (const QModelIndex& index) const
	{
		if (!index.isValid ())
			return {};

		return Bookmarks_ [index.row ()];
	}

	void DocumentBookmarksModel::AddBookmark (const Bookmark& bookmark, ListPasskey)
	{
		beginInsertRows ({}, Bookmarks_.size (), Bookmarks_.size ());
		Bookmarks_ << bookmark;
		endInsertRows ();
	}

	void DocumentBookmarksModel::RemoveBookmark (const Bookmark& bookmark, ListPasskey)
	{
		const auto idx = Bookmarks_.indexOf (bookmark);
		if (idx < 0)
		{
			qWarning () << "no bookmark" << bookmark << "found";
			return;
		}

		beginRemoveRows ({}, idx, idx);
		Bookmarks_.removeAt (idx);
		endRemoveRows ();
	}
}
