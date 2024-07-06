/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "documentbookmarksmanager.h"
#include <QStandardItemModel>
#include <util/sll/prelude.h>
#include "core.h"
#include "bookmarksmanager.h"
#include "bookmark.h"
#include "documenttab.h"

namespace LC::Monocle
{
	namespace
	{
		enum Roles
		{
			RBookmark = Qt::UserRole + 1
		};
	}

	DocumentBookmarksManager::DocumentBookmarksManager (DocumentTab *tab, QObject *parent)
	: QObject { parent }
	, Tab_ { tab }
	, Model_ { new QStandardItemModel { this } }
	{
	}

	QAbstractItemModel* DocumentBookmarksManager::GetModel () const
	{
		return Model_;
	}

	bool DocumentBookmarksManager::HasDoc () const
	{
		return Doc_ != nullptr;
	}

	void DocumentBookmarksManager::HandleDoc (IDocument_ptr doc)
	{
		Doc_ = doc;
		ReloadBookmarks ();

		emit docAvailable (HasDoc ());
	}

	void DocumentBookmarksManager::AddBookmark ()
	{
		if (!Doc_)
			return;

		/* TODO this is broken, ensure relative center is stored
		const auto page = Tab_->GetCurrentPage ();
		const auto& center = Tab_->GetCurrentCenter ();

		Bookmark bm (tr ("Page %1").arg (page + 1), page, center);
		Core::Instance ().GetBookmarksManager ()->AddBookmark (Doc_, bm);
		 */

		ReloadBookmarks ();
	}

	void DocumentBookmarksManager::RemoveBookmark (QModelIndex idx)
	{
		if (!idx.isValid ())
			return;

		const auto& bm = idx.data (Roles::RBookmark).value<Bookmark> ();
		Core::Instance ().GetBookmarksManager ()->RemoveBookmark (*Doc_, bm);

		ReloadBookmarks ();
	}

	void DocumentBookmarksManager::Navigate (const QModelIndex& idx)
	{
		const auto& bm = idx.data (Roles::RBookmark).value<Bookmark> ();
		// TODO this is broken anyway, store page instead of position Tab_->CenterOn (bm.GetPosition ());
	}

	void DocumentBookmarksManager::ReloadBookmarks ()
	{
		Model_->clear ();
		Model_->setHorizontalHeaderLabels ({ tr ("Name") });

		if (!Doc_)
			return;

		auto bookmarks = Core::Instance ().GetBookmarksManager ()->GetBookmarks (*Doc_);
		std::sort (bookmarks.begin (), bookmarks.end (),
				Util::ComparingBy ([] (const Bookmark& bm)
				{
					const auto& pos = bm.Position_;
					return std::make_tuple (pos.x (), pos.y ());
				}));

		for (const auto& bm : bookmarks)
		{
			auto item = new QStandardItem (bm.Name_);
			item->setEditable (false);
			item->setData (QVariant::fromValue<Bookmark> (bm), Roles::RBookmark);
			Model_->appendRow (item);
		}
	}
}
