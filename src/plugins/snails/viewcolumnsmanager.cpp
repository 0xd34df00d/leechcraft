/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "viewcolumnsmanager.h"
#include <memory>
#include <QHeaderView>
#include <QEvent>
#include <QtDebug>

namespace LC
{
namespace Snails
{
	ViewColumnsManager::ViewColumnsManager (QHeaderView *view)
	: QObject { view }
	, View_ { view}
	{
		View_->setStretchLastSection (false);
		connect (View_,
				SIGNAL (sectionCountChanged (int, int)),
				this,
				SLOT (handleSectionCountChanged (int, int)));
		connect (View_,
				SIGNAL (sectionResized (int, int, int)),
				this,
				SLOT (handleSectionResized (int, int, int)));

		View_->installEventFilter (this);
	}

	void ViewColumnsManager::SetStretchColumn (int column)
	{
		StretchColumn_ = column;
	}

	void ViewColumnsManager::SetDefaultWidth (int idx, int width)
	{
		ColumnWidths_ [idx] = width;
	}

	void ViewColumnsManager::SetDefaultWidths (const QList<int>& widths)
	{
		ColumnWidths_ = widths;
	}

	void ViewColumnsManager::SetDefaultWidths (const QStringList& strings)
	{
		auto font = View_->font ();
		font.setBold (true);
		const QFontMetrics fm { font };

		QList<int> widths;

		for (const auto& string : strings)
			widths << fm.horizontalAdvance (string);

		SetDefaultWidths (widths);
	}

	void ViewColumnsManager::SetSwaps (const QList<QPair<int, int>>& swaps)
	{
		Swaps_ = swaps;
	}

	bool ViewColumnsManager::eventFilter (QObject *object, QEvent *event)
	{
		if (event->type () == QEvent::Resize)
			readjustWidths ();

		return QObject::eventFilter (object, event);
	}

	void ViewColumnsManager::readjustWidths ()
	{
		IgnoreResizes_ = true;
		const std::shared_ptr<void> ignoreGuard
		{
			nullptr,
			[this] (void*) { IgnoreResizes_ = false; }
		};

		const auto scrollBarWidth = View_->style ()->pixelMetric (QStyle::PM_ScrollBarExtent);
		auto remainingWidth = View_->width () - scrollBarWidth;

		for (int i = 0; i < ColumnWidths_.size (); ++i)
		{
			if (i == StretchColumn_)
				continue;

			View_->resizeSection (i, ColumnWidths_.at (i));
			remainingWidth -= ColumnWidths_.at (i);
		}

		if (StretchColumn_ >= 0)
		{
			remainingWidth = std::max (20, remainingWidth);
			View_->resizeSection (StretchColumn_, remainingWidth);
		}
	}

	void ViewColumnsManager::handleSectionResized (int index, int, int newSize)
	{
		if (IgnoreResizes_)
			return;

		if (index == StretchColumn_)
			StretchColumn_ = -1;

		ColumnWidths_ [index] = newSize;
	}

	void ViewColumnsManager::handleSectionCountChanged (int, int newCount)
	{
		if (newCount != ColumnWidths_.size ())
			return;

		readjustWidths ();

		for (const auto& pair : Swaps_)
			View_->swapSections (pair.first, pair.second);
	}
}
}
