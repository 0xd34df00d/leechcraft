/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#include "flowlayout.h"
#include <QWidget>

namespace LeechCraft
{
namespace Util
{
	FlowLayout::FlowLayout (QWidget *parent,
			int margin, int hspace, int vspace)
	: QLayout (parent)
	, HSpace_ (hspace)
	, VSpace_ (vspace)
	{
		setContentsMargins (margin, margin, margin, margin);
	}

	FlowLayout::FlowLayout (int margin, int hspace, int vspace)
	: HSpace_ (hspace)
	, VSpace_ (vspace)
	{
		setContentsMargins (margin, margin, margin, margin);
	}

	FlowLayout::~FlowLayout ()
	{
		QLayoutItem *item = 0;
		while ((item = takeAt (0)))
			delete item;
	}

	void FlowLayout::addItem (QLayoutItem *item)
	{
		ItemList_ << item;
	}

	int FlowLayout::horizontalSpacing () const
	{
		return HSpace_ >= 0 ?
				HSpace_ :
				SmartSpacing (QStyle::PM_LayoutHorizontalSpacing);
	}

	int FlowLayout::verticalSpacing () const
	{
		return VSpace_ >= 0 ?
				VSpace_ :
				SmartSpacing (QStyle::PM_LayoutVerticalSpacing);
	}

	Qt::Orientations FlowLayout::expandingDirections () const
	{
		return 0;
	}

	bool FlowLayout::hasHeightForWidth () const
	{
		return true;
	}

	int FlowLayout::heightForWidth (int width) const
	{
		return DoLayout (QRect (0, 0, width, 0), true);
	}

	int FlowLayout::count () const
	{
		return ItemList_.size ();
	}

	QLayoutItem* FlowLayout::itemAt (int idx) const
	{
		return ItemList_.value (idx);
	}

	QLayoutItem* FlowLayout::takeAt (int idx)
	{
		if (idx >= 0 && idx < ItemList_.size ())
			return ItemList_.takeAt (idx);
		else
			return 0;
	}

	QSize FlowLayout::minimumSize () const
	{
		QSize size;
		Q_FOREACH (const QLayoutItem *item, ItemList_)
			size = size.expandedTo (item->minimumSize ());

		size += QSize (margin () * 2, margin () * 2);
		return size;
	}

	void FlowLayout::setGeometry (const QRect& rect)
	{
		QLayout::setGeometry (rect);
		DoLayout (rect, false);
	}

	QSize FlowLayout::sizeHint () const
	{
		return minimumSize ();
	}

	int FlowLayout::DoLayout (const QRect& rect, bool testOnly) const
	{
		int left = 0, top = 0, right = 0, bottom = 0;
		getContentsMargins (&left, &top, &right, &bottom);

		const QRect& effectiveRect = rect.adjusted (left, top, -right, -bottom);
		int x = effectiveRect.x ();
		int y = effectiveRect.y ();
		int lineHeight = 0;

		Q_FOREACH (QLayoutItem *item, ItemList_)
		{
			QWidget *widget = item->widget ();

			int spaceX = horizontalSpacing ();
			if (spaceX == -1)
				spaceX = widget->style ()->layoutSpacing (QSizePolicy::PushButton,
						QSizePolicy::PushButton, Qt::Horizontal);
			int spaceY = verticalSpacing ();
			if (spaceY == -1)
				spaceY = widget->style ()->layoutSpacing (QSizePolicy::PushButton,
						QSizePolicy::PushButton, Qt::Vertical);

			int nextX = x + item->sizeHint ().width () + spaceX;
			if (nextX - spaceX > effectiveRect.right () &&
					lineHeight > 0)
			{
				x = effectiveRect.x ();
				y += lineHeight + spaceY;
				nextX = x + item->sizeHint ().width () + spaceX;
				lineHeight = 0;
			}

			if (!testOnly)
				item->setGeometry (QRect (QPoint (x, y), item->sizeHint ()));

			x = nextX;
			lineHeight = std::max (lineHeight, item->sizeHint ().height ());
		}

		return y + lineHeight - rect.y () + bottom;
	}

	int FlowLayout::SmartSpacing (QStyle::PixelMetric pm) const
	{
		QObject *obj = parent ();
		if (!obj)
			return -1;
		else if (obj->isWidgetType ())
		{
			QWidget *pw = static_cast<QWidget*> (obj);
			return pw->style ()->pixelMetric (pm, 0, pw);
		}
		else
			return static_cast<QLayout*> (obj)->spacing ();
	}
}
}
