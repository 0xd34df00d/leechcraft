/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "pagesview.h"
#include <QGraphicsItem>
#include <QMouseEvent>
#include "interfaces/monocle/idocument.h"

namespace LC::Monocle
{
	void PagesView::SetDocument (IDocument *doc)
	{
		Doc_ = doc;
	}

	void PagesView::CenterOn (SceneAbsolutePos p)
	{
		centerOn (p.ToPointF ());
	}

	SceneAbsolutePos PagesView::GetCurrentCenter () const
	{
		const auto& rectSize = viewport ()->contentsRect ().size () / 2;
		return SceneAbsolutePos { mapToScene (QPoint { rectSize.width (), rectSize.height () }) };
	}

	SceneAbsolutePos PagesView::GetViewportTrimmedCenter (const QGraphicsItem& item) const
	{
		auto center = item.boundingRect ().bottomRight ();
		center.ry () = std::min (center.y (), static_cast<qreal> (viewport ()->contentsRect ().height ()));
		center /= 2;
		return SceneAbsolutePos { item.mapToScene (center) };
	}

	void PagesView::mousePressEvent (QMouseEvent *event)
	{
		QGraphicsView::mousePressEvent (event);
		if (Doc_)
			InteractionHandler_->Pressed (*event, *Doc_);
	}

	void PagesView::mouseMoveEvent (QMouseEvent *event)
	{
		QGraphicsView::mouseMoveEvent (event);
		if (Doc_)
			InteractionHandler_->Moved (*event, *Doc_);
	}

	void PagesView::mouseReleaseEvent (QMouseEvent *event)
	{
		QGraphicsView::mouseReleaseEvent (event);
		if (Doc_)
			InteractionHandler_->Released (*event, *Doc_);
	}

	void PagesView::resizeEvent (QResizeEvent *e)
	{
		QGraphicsView::resizeEvent (e);
		emit sizeChanged ();
	}
}
