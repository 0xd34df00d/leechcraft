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
	{
		Ui_.setupUi (this);
		Ui_.MainLayout_->insertWidget (0, Toolbar_);

		Toolbar_->addAction (tr ("Add bookmark"),
				this, SLOT (handleAddBookmark ()));
	}

	void BookmarksWidget::HandleDoc (IDocument_ptr doc)
	{
		setEnabled (doc == nullptr);

		Doc_ = doc;
	}

	void BookmarksWidget::handleAddBookmark ()
	{
		if (!Doc_)
			return;

		const auto page = Tab_->GetCurrentPage ();
		const auto& center = Tab_->GetCurrentCenter ();

		Bookmark bm (tr ("Page %1"), page, center);
		Core::Instance ().GetBookmarksManager ()->AddBookmark (Doc_, bm);
	}
}
}
