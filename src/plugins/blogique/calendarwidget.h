/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QCalendarWidget>
#include <QMap>

namespace LC
{
namespace Blogique
{
	class CalendarWidget : public QCalendarWidget
	{
		Q_OBJECT

		QMap<QDate, int> Date2EntriesCount_;
	public:
		CalendarWidget (QWidget *parent = 0);

		void SetStatistic (const QMap<QDate, int>& statistic);
	protected:
#if QT_VERSION_MAJOR >= 6
		void paintCell (QPainter *painter, const QRect& rect, QDate date) const override;
#else
		void paintCell (QPainter *painter, const QRect& rect, const QDate& date) const;
#endif
	};
}
}
