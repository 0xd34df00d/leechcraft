/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "pagesview.h"
#include <QMenu>
#include <QMouseEvent>
#include "documenttab.h"

namespace LC::Monocle
{
	void PagesView::SetDocumentTab (DocumentTab *tab)
	{
		DocTab_ = tab;
	}

	void PagesView::SetShowReleaseMenu (bool show)
	{
		ShowReleaseMenu_ = show;
		ShowOnNextRelease_ = false;
	}

	QPointF PagesView::GetCurrentCenter () const
	{
		const auto& rectSize = viewport ()->contentsRect ().size () / 2;
		return mapToScene (QPoint (rectSize.width (), rectSize.height ()));
	}

	void PagesView::mouseMoveEvent (QMouseEvent *event)
	{
		if (event->buttons () != Qt::NoButton && ShowReleaseMenu_)
			ShowOnNextRelease_ = true;

		QGraphicsView::mouseMoveEvent (event);
	}

	void PagesView::mouseReleaseEvent (QMouseEvent *event)
	{
		QGraphicsView::mouseReleaseEvent (event);

		if (!ShowOnNextRelease_)
			return;

		auto menu = new QMenu { this };
		DocTab_->CreateViewCtxMenuActions (menu);
		menu->popup (event->globalPos ());
		menu->setAttribute (Qt::WA_DeleteOnClose);
		menu->show ();

		ShowOnNextRelease_ = false;
	}

	void PagesView::resizeEvent (QResizeEvent *e)
	{
		QGraphicsView::resizeEvent (e);
		emit sizeChanged ();
	}
}
