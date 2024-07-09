/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "bookmarksstorage.h"
#include <QDir>
#include <QFileInfo>
#include <QSqlError>
#include <util/db/util.h>
#include <util/db/oral/oral.h>
#include <util/sys/paths.h>
#include <util/sll/qtutil.h>
#include "interfaces/monocle/idocument.h"
#include "documentbookmarksmodel.h"

namespace LC::Monocle
{
	namespace oral = Util::oral;
	namespace sph = Util::oral::sph;

	struct BookmarksStorage::BookmarkRecord
	{
		oral::NotNull<QString> DocId_;
		oral::NotNull<QString> Name_;
		oral::NotNull<int> Page_;
		oral::NotNull<double> X_;
		oral::NotNull<double> Y_;

		constexpr static auto ClassName = "Bookmark"_ct;
	};
}

ORAL_ADAPT_STRUCT (LC::Monocle::BookmarksStorage::BookmarkRecord,
		DocId_,
		Name_,
		Page_,
		X_,
		Y_)

namespace LC::Monocle
{
	BookmarksStorage::BookmarksStorage (QObject *parent)
	: QObject { parent }
	, Db_ { Util::OpenSqliteDatabase ({
				.Connection_ = "org.LeechCraft.Monocle.BookmarksStorage"_qs,
				.DirKind_ = Util::UserDir::LC,
				.Dir_ = "monocle"_qs,
				.Filename_ = "bookmarks.db"_qs
			})
		}
	, Bookmarks_ { oral::AdaptPtr<BookmarkRecord> (Db_) }
	{
	}

	BookmarksStorage::~BookmarksStorage () = default;

	namespace
	{
		QString GetDocID (const IDocument& doc)
		{
			const auto& info = doc.GetDocumentInfo ();
			if (!info.Title_.trimmed ().isEmpty ())
				return info.Title_;

			return QFileInfo { doc.GetDocURL ().path () }.fileName ();
		}
	}

	std::shared_ptr<DocumentBookmarksModel> BookmarksStorage::GetDocumentBookmarksModel (const IDocument& doc)
	{
		const auto& id = GetDocID (doc);

		auto& maybeModel = Id2Model_ [id];
		if (const auto model = maybeModel.lock ())
			return model;

		auto model = std::make_shared<DocumentBookmarksModel> (LoadBookmarks (id));
		maybeModel = model;
		return model;
	}

	void BookmarksStorage::AddBookmark (const IDocument& doc, const Bookmark& bookmark)
	{
		const auto& id = GetDocID (doc);
		SaveBookmark (id, bookmark);
		if (const auto model = Id2Model_ [id].lock ())
			model->AddBookmark (bookmark, {});
	}

	void BookmarksStorage::RemoveBookmark (const IDocument& doc, const Bookmark& bookmark)
	{
		const auto& id = GetDocID (doc);
		DeleteBookmark (id, bookmark);
		if (const auto model = Id2Model_ [id].lock ())
			model->RemoveBookmark (bookmark, {});
	}

	namespace
	{
		Bookmark ToBookmark (const BookmarksStorage::BookmarkRecord& r)
		{
			return { .Name_ = r.Name_, .Page_ = r.Page_, .Position_ = QPointF { r.X_, r.Y_ } };
		}

		BookmarksStorage::BookmarkRecord FromBookmark (const QString& id, const Bookmark& bm)
		{
			const auto& pos = bm.Position_;
			return { .DocId_ = id, .Name_ = bm.Name_, .Page_ = bm.Page_, .X_ = pos.x (), .Y_ = pos.y () };
		}
	}

	QVector<Bookmark> BookmarksStorage::LoadBookmarks (const QString& id)
	{
		return Util::MapAs<QVector> (Bookmarks_->Select (sph::f<&BookmarkRecord::DocId_> == id), &ToBookmark);
	}

	void BookmarksStorage::SaveBookmark (const QString& id, const Bookmark& bookmark)
	{
		Bookmarks_->Insert (FromBookmark (id, bookmark));
	}

	void BookmarksStorage::DeleteBookmark (const QString& id, const Bookmark& bookmark)
	{
		// name omitted deliberately
		Bookmarks_->DeleteBy (sph::f<&BookmarkRecord::DocId_> == id &&
				sph::f<&BookmarkRecord::Page_> == bookmark.Page_ &&
				sph::f<&BookmarkRecord::X_> == bookmark.Position_.x () &&
				sph::f<&BookmarkRecord::Y_> == bookmark.Position_.y ());
	}
}
