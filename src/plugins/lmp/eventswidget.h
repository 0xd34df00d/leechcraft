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

#pragma once

#include <QWidget>
#include "ui_eventswidget.h"

class QStandardItemModel;

namespace Media
{
	class IEventsProvider;

	struct EventInfo;
	typedef QList<EventInfo> EventInfos_t;
}

namespace LeechCraft
{
namespace LMP
{
	class EventsWidget : public QWidget
	{
		Q_OBJECT

		Ui::EventsWidget Ui_;
		QStandardItemModel *Model_;
		QList<Media::IEventsProvider*> Providers_;
	public:
		EventsWidget (QWidget* = 0);

		void InitializeProviders ();
	private slots:
		void on_Provider__activated (int);
		void handleEvents (const Media::EventInfos_t&);
		void handleAttendSure (int);
		void handleAttendMaybe (int);
		void handleUnattend (int);
	};
}
}
