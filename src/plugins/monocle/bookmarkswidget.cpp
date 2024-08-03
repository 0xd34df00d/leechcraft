/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "bookmarkswidget.h"
#include "components/navigation/bookmarksstorage.h"
#include "components/navigation/documentbookmarksmodel.h"
#include "components/services/linkexecutioncontext.h"

namespace LC::Monocle
{
	BookmarksWidget::BookmarksWidget (BookmarksStorage& storage, QWidget *parent)
	: QWidget { parent }
	, Storage_ { storage }
	{
		Ui_.setupUi (this);
		Ui_.MainLayout_->insertWidget (0, &Toolbar_);

		setEnabled (false);

		auto addBm = Toolbar_.addAction (tr ("Add bookmark"),
				this,
				&BookmarksWidget::addBookmarkRequested);
		addBm->setProperty ("ActionIcon", "bookmark-new");

		auto removeBookmark = Toolbar_.addAction (tr ("Remove bookmark"),
				this,
				[this] { emit removeBookmarkRequested (Model_->GetBookmark (Ui_.BookmarksView_->currentIndex ())); });
		removeBookmark->setProperty ("ActionIcon", "list-remove");
		Ui_.BookmarksView_->addAction (removeBookmark);

		connect (Ui_.BookmarksView_,
				&QTreeView::activated,
				this,
				[this] (const QModelIndex& index) { emit bookmarkActivated (Model_->GetBookmark (index)); });
	}

	void BookmarksWidget::HandleDoc (const IDocument& doc)
	{
		setEnabled (true);
		Model_ = Storage_.GetDocumentBookmarksModel (doc);
		Ui_.BookmarksView_->setModel (&*Model_);
	}
}
