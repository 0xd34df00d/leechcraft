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
#include "ui_bookmarkswidget.h"

namespace LC::Monocle
{
	class DocumentBookmarksManager;

	class BookmarksWidget : public QWidget
	{
		Ui::BookmarksWidget Ui_;
		QToolBar Toolbar_;
	public:
		explicit BookmarksWidget (DocumentBookmarksManager&, QWidget* = nullptr);
	};
}
