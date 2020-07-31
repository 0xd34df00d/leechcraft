/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "bookmarkswidget.h"
#include <QToolBar>
#include "documentbookmarksmanager.h"

namespace LC
{
namespace Monocle
{
	BookmarksWidget::BookmarksWidget (DocumentBookmarksManager *mgr, QWidget *parent)
	: QWidget (parent)
	, Toolbar_ (new QToolBar)
	{
		Ui_.setupUi (this);
		Ui_.BookmarksView_->setModel (mgr->GetModel ());
		Ui_.MainLayout_->insertWidget (0, Toolbar_);

		setEnabled (mgr->HasDoc ());
		connect (mgr,
				&DocumentBookmarksManager::docAvailable,
				this,
				&QWidget::setEnabled);

		auto addBm = Toolbar_->addAction (tr ("Add bookmark"),
				mgr, &DocumentBookmarksManager::AddBookmark);
		addBm->setProperty ("ActionIcon", "bookmark-new");

		auto removeBookmark = Toolbar_->addAction (tr ("Remove bookmark"),
				mgr,
				[mgr, this] { mgr->RemoveBookmark (Ui_.BookmarksView_->currentIndex ()); });
		removeBookmark->setProperty ("ActionIcon", "list-remove");
		Ui_.BookmarksView_->addAction (removeBookmark);

		connect (Ui_.BookmarksView_,
				&QTreeView::activated,
				mgr,
				&DocumentBookmarksManager::Navigate);
	}
}
}
