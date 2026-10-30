/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QGraphicsObject>
#include <QPainterPath>
#include "components/layout/positions.h"

namespace LC::Monocle
{
	class PageGraphicsItem;

	class TextSelectionItem final : public QGraphicsObject
	{
		QPainterPath Path_;
	public:
		explicit TextSelectionItem (PageGraphicsItem&);

		void SetRects (const QVector<PageRelativeRect>&);

		QRectF boundingRect () const override;
		void paint (QPainter*, const QStyleOptionGraphicsItem*, QWidget*) override;
	};
}
