/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "flowlayout.h"
#include <QWidget>

namespace LC::Util
{
	FlowLayout::FlowLayout (QWidget *parent,
			int margin, int hspace, int vspace)
	: QLayout { parent }
	, HSpace_ { hspace }
	, VSpace_ { vspace }
	{
		setContentsMargins (margin, margin, margin, margin);
	}

	FlowLayout::FlowLayout (int margin, int hspace, int vspace)
	: FlowLayout { nullptr, margin, hspace, vspace }
	{
	}

	FlowLayout::~FlowLayout ()
	{
		qDeleteAll (ItemList_);
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
		return {};
	}

	bool FlowLayout::hasHeightForWidth () const
	{
		return true;
	}

	int FlowLayout::heightForWidth (int width) const
	{
		return DoLayout ({ 0, 0, width, 0 }, true);
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
		if (idx < 0 || idx >= ItemList_.size ())
			return nullptr;

		return ItemList_.takeAt (idx);
	}

	QSize FlowLayout::minimumSize () const
	{
		QSize size;
		for (const auto item : ItemList_)
			size = size.expandedTo (item->minimumSize ());

		int left = 0;
		int top = 0;
		int right = 0;
		int bottom = 0;
		getContentsMargins (&left, &top, &right, &bottom);
		size += QSize { left + right, top + bottom };
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

		const auto& effectiveRect = rect.adjusted (left, top, -right, -bottom);
		int x = effectiveRect.x ();
		int y = effectiveRect.y ();
		int lineHeight = 0;

		for (const auto item : ItemList_)
		{
			const auto widget = item->widget ();

			int spaceX = horizontalSpacing ();
			if (spaceX == -1)
				spaceX = widget->style ()->layoutSpacing (QSizePolicy::PushButton,
						QSizePolicy::PushButton, Qt::Horizontal);
			int spaceY = verticalSpacing ();
			if (spaceY == -1)
				spaceY = widget->style ()->layoutSpacing (QSizePolicy::PushButton,
						QSizePolicy::PushButton, Qt::Vertical);

			const auto& sizeHint = item->sizeHint ();
			const int hintWidth = sizeHint.width ();
			int nextX = x + hintWidth + spaceX;
			if (nextX - spaceX > effectiveRect.right () &&
					lineHeight > 0)
			{
				x = effectiveRect.x ();
				y += lineHeight + spaceY;
				nextX = x + hintWidth + spaceX;
				lineHeight = 0;
			}

			if (!testOnly)
				item->setGeometry ({ { x, y }, sizeHint });

			x = nextX;
			lineHeight = std::max (lineHeight, sizeHint.height ());
		}

		return y + lineHeight - rect.y () + bottom;
	}

	int FlowLayout::SmartSpacing (QStyle::PixelMetric pm) const
	{
		const auto obj = parent ();
		if (!obj)
			return -1;

		if (const auto pw = dynamic_cast<QWidget*> (obj))
			return pw->style ()->pixelMetric (pm, nullptr, pw);
		if (const auto lay = dynamic_cast<QLayout*> (obj))
			return lay->spacing ();

		return -1;
	}
}
