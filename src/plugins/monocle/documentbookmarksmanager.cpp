/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "documentbookmarksmanager.h"
#include <QStandardItemModel>
#include <QMenu>
#include <util/sll/prelude.h>
#include "core.h"
#include "bookmarksmanager.h"
#include "bookmark.h"
#include "documenttab.h"

namespace LC
{
namespace Monocle
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
	, Menu_ { new QMenu }
	{
	}

	QAbstractItemModel* DocumentBookmarksManager::GetModel () const
	{
		return Model_;
	}

	QMenu* DocumentBookmarksManager::GetMenu () const
	{
		return Menu_;
	}

	bool DocumentBookmarksManager::HasDoc () const
	{
		return Doc_ != nullptr;
	}

	void DocumentBookmarksManager::HandleDoc (IDocument_ptr doc)
	{
		Doc_ = doc;
		ReloadBookmarks ();

		const auto hasDoc = HasDoc ();
		Menu_->setEnabled (hasDoc);
		emit docAvailable (hasDoc);
	}

	void DocumentBookmarksManager::AddBookmark ()
	{
		if (!Doc_)
			return;

		const auto page = Tab_->GetCurrentPage ();
		const auto& center = Tab_->GetCurrentCenter ();

		Bookmark bm (tr ("Page %1").arg (page + 1), page, center);
		Core::Instance ().GetBookmarksManager ()->AddBookmark (Doc_, bm);

		ReloadBookmarks ();
	}

	void DocumentBookmarksManager::RemoveBookmark (QModelIndex idx)
	{
		if (!idx.isValid ())
			return;

		idx = idx.sibling (idx.row (), 0);
		const auto& bm = idx.data (Roles::RBookmark).value<Bookmark> ();
		Core::Instance ().GetBookmarksManager ()->RemoveBookmark (Doc_, bm);

		ReloadBookmarks ();
	}

	void DocumentBookmarksManager::Navigate (const QModelIndex& idx)
	{
		const auto& bm = idx.sibling (idx.row (), 0).data (Roles::RBookmark).value<Bookmark> ();
		Tab_->CenterOn (bm.GetPosition ());
	}

	void DocumentBookmarksManager::ReloadBookmarks ()
	{
		Model_->clear ();
		Model_->setHorizontalHeaderLabels ({ tr ("Name") });

		Menu_->clear ();

		if (!Doc_)
			return;

		auto bookmarks = Core::Instance ().GetBookmarksManager ()->GetBookmarks (Doc_);
		std::sort (bookmarks.begin (), bookmarks.end (),
				Util::ComparingBy ([] (const Bookmark& bm)
				{
					const auto& pos = bm.GetPosition ();
					return std::make_tuple (pos.x (), pos.y ());
				}));

		for (const auto& bm : bookmarks)
		{
			auto item = new QStandardItem (bm.GetName ());
			item->setEditable (false);
			item->setData (QVariant::fromValue<Bookmark> (bm), Roles::RBookmark);
			Model_->appendRow (item);

			Menu_->addAction (bm.GetName (),
					this,
					[this, bm] { Tab_->CenterOn (bm.GetPosition ()); });
		}

		Menu_->addSeparator ();
		Menu_->addAction (QIcon::fromTheme ("bookmark-new"),
				tr ("Add bookmark"),
				this,
				&DocumentBookmarksManager::AddBookmark);
	}
}
}
