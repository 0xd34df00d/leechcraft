/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QToolBar>
#include <QWidget>
#include "components/layout/positions.h"
#include "ui_bookmarkswidget.h"

namespace LC::Monocle
{
	struct Bookmark;
	class BookmarksStorage;
	class DocumentBookmarksModel;
	class IDocument;

	struct LinkExecutionContext;

	class BookmarksWidget : public QWidget
	{
		Q_OBJECT

		Ui::BookmarksWidget Ui_;
		QToolBar Toolbar_;

		std::shared_ptr<DocumentBookmarksModel> Model_;
		BookmarksStorage& Storage_;
	public:
		explicit BookmarksWidget (BookmarksStorage&, QWidget* = nullptr);

		void HandleDoc (const IDocument&);
	signals:
		void addBookmarkRequested ();
		void removeBookmarkRequested (const Bookmark&);
		void bookmarkActivated (const Bookmark&);
	};
}
