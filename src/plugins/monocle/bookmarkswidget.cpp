/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "bookmarkswidget.h"
#include <QToolBar>
#include <QStandardItemModel>
#include "documenttab.h"
#include "bookmark.h"
#include "core.h"
#include "bookmarksmanager.h"

namespace LeechCraft
{
namespace Monocle
{
	BookmarksWidget::BookmarksWidget (DocumentTab *tab, QWidget *parent)
	: QWidget (parent)
	, Tab_ (tab)
	, Toolbar_ (new QToolBar ())
	, BMModel_ (new QStandardItemModel (this))
	{
		Ui_.setupUi (this);
		Ui_.BookmarksView_->setModel (BMModel_);
		Ui_.MainLayout_->insertWidget (0, Toolbar_);

		HandleDoc ({});

		auto addBm = Toolbar_->addAction (tr ("Add bookmark"),
				this, SLOT (handleAddBookmark ()));
		addBm->setProperty ("ActionIcon", "bookmark-new");
	}

	void BookmarksWidget::HandleDoc (IDocument_ptr doc)
	{
		setEnabled (doc != nullptr);
		Doc_ = doc;

		BMModel_->clear ();
		BMModel_->setHorizontalHeaderLabels ({ tr ("Name") });

		if (!doc)
			return;

		for (const auto& bm : Core::Instance ().GetBookmarksManager ()->GetBookmarks (doc))
			AddBMToTree (bm);
	}

	void BookmarksWidget::AddBMToTree (const Bookmark& bm)
	{
		auto item = new QStandardItem (bm.GetName ());
		item->setEditable (false);
		BMModel_->appendRow (item);
	}

	void BookmarksWidget::handleAddBookmark ()
	{
		if (!Doc_)
			return;

		const auto page = Tab_->GetCurrentPage ();
		const auto& center = Tab_->GetCurrentCenter ();

		Bookmark bm (tr ("Page %1").arg (page + 1), page, center);
		Core::Instance ().GetBookmarksManager ()->AddBookmark (Doc_, bm);
		AddBMToTree (bm);
	}
}
}
