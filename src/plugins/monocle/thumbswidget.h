/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include <util/gui/uiinit.h>
#include "ui_thumbswidget.h"

namespace LC::Monocle
{
	class IDocument;
	class PageGraphicsItem;
	class PagesLayoutManager;
	class SmoothScroller;

	class ThumbsWidget : public QWidget
	{
		Q_OBJECT

		Ui::ThumbsWidget Ui_;
		Util::UiInit UiInit_ { Ui_, *this };

		QGraphicsScene Scene_;
		SmoothScroller& Scroller_;

		PagesLayoutManager *LayoutMgr_;

		QVector<QGraphicsRectItem*> CurrentAreaRects_;
		QVector<PageGraphicsItem*> Pages_;
		QMap<int, QRect> LastVisibleAreas_;
	public:
		explicit ThumbsWidget (QWidget* = nullptr);

		void HandleDoc (IDocument*);

		void UpdatePagesVisibility (const QMap<int, QRect>&);
		void SetCurrentPage (int);
	signals:
		void pageClicked (int);
	};
}
