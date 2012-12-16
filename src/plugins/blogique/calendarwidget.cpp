/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
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

#include "calendarwidget.h"
#include <QPainter>

namespace LeechCraft
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
