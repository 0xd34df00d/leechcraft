/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QGraphicsView>

namespace LC::Monocle
{
	class DocumentTab;

	class PagesView : public QGraphicsView
	{
		Q_OBJECT

		bool ShowReleaseMenu_ = false;
		bool ShowOnNextRelease_ = false;

		DocumentTab *DocTab_ = nullptr;
	public:
		using QGraphicsView::QGraphicsView;

		void SetDocumentTab (DocumentTab*);
		void SetShowReleaseMenu (bool);

		QPointF GetCurrentCenter () const;
	protected:
		void mouseMoveEvent (QMouseEvent*) override;
		void mouseReleaseEvent (QMouseEvent*) override;
		void resizeEvent (QResizeEvent*) override;
	signals:
		void sizeChanged ();
	};
}
