/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#pragma once

#include <functional>
#include <QGraphicsPixmapItem>
#include "interfaces/monocle/idocument.h"

namespace LeechCraft
{
namespace Monocle
{
	class PageGraphicsItem : public QGraphicsPixmapItem
	{
		IDocument_ptr Doc_;
		const int PageNum_;

		bool IsHoverLink_;
		QList<ILink_ptr> Links_;
		QList<QPair<QRect, ILink_ptr>> Rect2Link_;
		ILink_ptr PressedLink_;

		double XScale_;
		double YScale_;

		bool Invalid_;

		std::function<void (int, QPointF)> ReleaseHandler_;
	public:
		PageGraphicsItem (IDocument_ptr, int, QGraphicsItem* = 0);
		~PageGraphicsItem ();

		void SetReleaseHandler (std::function<void (int, QPointF)>);

		void SetScale (double, double);
		int GetPageNum () const;

		QRectF MapFromDoc (const QRectF&) const;
		QRectF MapToDoc (const QRectF&) const;

		void ClearPixmap ();
		void UpdatePixmap ();
	protected:
		void paint (QPainter*, const QStyleOptionGraphicsItem*, QWidget*);
		void hoverMoveEvent (QGraphicsSceneHoverEvent*);
		void hoverLeaveEvent (QGraphicsSceneHoverEvent*);
		void mousePressEvent (QGraphicsSceneMouseEvent*);
		void mouseReleaseEvent (QGraphicsSceneMouseEvent*);
	private:
		void LayoutLinks ();
		ILink_ptr FindLink (const QPointF&);
	};
}
}
