/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include "ui_thumbswidget.h"

namespace LC::Monocle
{
	class IDocument;
	class PagesLayoutManager;

	class ThumbsWidget : public QWidget
	{
		Q_OBJECT

		Ui::ThumbsWidget Ui_;
		QGraphicsScene Scene_;

		PagesLayoutManager *LayoutMgr_;

		QList<QGraphicsRectItem*> CurrentAreaRects_;
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
