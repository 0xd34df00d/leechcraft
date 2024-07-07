/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QAbstractItemModel>
#include <QCoreApplication>
#include "bookmark.h"

namespace LC::Monocle
{
	class IDocument;

	class DocumentBookmarksModel : public QAbstractItemModel
	{
		Q_DECLARE_TR_FUNCTIONS (LC::Monocle::DocumentBookmarksModel)

		QVector<Bookmark> Bookmarks_;

		class ListPasskey
		{
			friend class BookmarksStorage;
			ListPasskey () = default;
		};
	public:
		explicit DocumentBookmarksModel (QVector<Bookmark>, QObject* = nullptr);

		QModelIndex index (int, int, const QModelIndex&) const override;
		QModelIndex parent (const QModelIndex&) const override;
		int rowCount (const QModelIndex&) const override;
		int columnCount (const QModelIndex&) const override;
		QVariant data (const QModelIndex&, int) const override;
		QVariant headerData (int, Qt::Orientation, int) const override;

		Bookmark GetBookmark (const QModelIndex&) const;

		void AddBookmark (const Bookmark&, ListPasskey);
		void RemoveBookmark (const Bookmark&, ListPasskey);
	};
}
