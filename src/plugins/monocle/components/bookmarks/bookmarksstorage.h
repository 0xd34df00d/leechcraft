/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QHash>
#include <QObject>
#include <QSqlDatabase>
#include <util/db/oral/oralfwd.h>

namespace LC::Monocle
{
	struct Bookmark;
	class DocumentBookmarksModel;
	class IDocument;

	class BookmarksStorage : public QObject
	{
		using WeakModelPtr = std::weak_ptr<DocumentBookmarksModel>;
		QHash<QString, WeakModelPtr> Id2Model_;

		QSqlDatabase Db_;
	public:
		struct BookmarkRecord;
	private:
		const Util::oral::ObjectInfo_ptr<BookmarkRecord> Bookmarks_;
	public:
		explicit BookmarksStorage (QObject* = nullptr);
		~BookmarksStorage () override;

		std::shared_ptr<DocumentBookmarksModel> GetDocumentBookmarksModel (const IDocument&);

		void AddBookmark (const IDocument&, const Bookmark&);
		void RemoveBookmark (const IDocument&, const Bookmark&);
	private:
		QVector<Bookmark> LoadBookmarks (const QString&);
		void SaveBookmark (const QString&, const Bookmark&);
		void DeleteBookmark (const QString&, const Bookmark&);
	};
}
