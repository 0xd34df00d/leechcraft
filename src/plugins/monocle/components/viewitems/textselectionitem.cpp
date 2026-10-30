/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "textselectionitem.h"
#include <QGuiApplication>
#include <QPainter>
#include <QWidget>
#include "pagegraphicsitem.h"

namespace LC::Monocle
{
	TextSelectionItem::TextSelectionItem (PageGraphicsItem& page)
	: QGraphicsObject { &page }
	{
		setZValue (1);
		setOpacity (0.5);
		setAcceptedMouseButtons (Qt::NoButton);

		page.RegisterChildRect (this,
				PageRelativeRect { PageRelativePos { 0, 0 }, PageRelativePos { 1, 1 } },
				[this] (const PageAbsoluteRect& pageRect)
				{
					const auto& size = pageRect.ToRectF ().size ();
					setTransform (QTransform::fromScale (size.width (), size.height ()));
				});
	}

	void TextSelectionItem::SetRects (const QVector<PageRelativeRect>& rects)
	{
		QPainterPath path;
		path.setFillRule (Qt::WindingFill);
		for (const auto& rect : rects)
			path.addRect (rect.ToRectF ());

		if (path == Path_)
			return;

		prepareGeometryChange ();
		Path_ = std::move (path);
		update ();
	}

	QRectF TextSelectionItem::boundingRect () const
	{
		return Path_.controlPointRect ();
	}

	void TextSelectionItem::paint (QPainter *painter, const QStyleOptionGraphicsItem*, QWidget *widget)
	{
		const auto& palette = widget ? widget->palette () : qGuiApp->palette ();
		painter->fillPath (Path_, palette.highlight ());
	}
}
