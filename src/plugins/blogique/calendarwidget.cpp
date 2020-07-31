/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "calendarwidget.h"
#include <QPainter>

namespace LC
{
namespace Blogique
{
	CalendarWidget::CalendarWidget (QWidget *parent)
	: QCalendarWidget (parent)
	{
	}

	void CalendarWidget::SetStatistic (const QMap<QDate, int>& statistic)
	{
		Date2EntriesCount_ = statistic;
		update ();
	}

	void CalendarWidget::paintCell (QPainter *painter, const QRect& rect, const QDate& date) const
	{
		QCalendarWidget::paintCell (painter, rect, date);
		
		if (Date2EntriesCount_.contains (date) &&
				Date2EntriesCount_ [date])
		{
			painter->save ();
			painter->setBrush (QBrush (Qt::blue));
			const QPointF points [3] =
			{
				QPointF (rect.x (), rect.bottom () - 8),
				QPointF (rect.x () + 8, rect.bottom ()),
				QPointF (rect.x (), rect.bottom ())
			};
			painter->drawPolygon (points, 3);
			painter->restore ();
		}
	}
}
}
