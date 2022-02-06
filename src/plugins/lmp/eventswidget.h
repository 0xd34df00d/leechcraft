/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include "ui_eventswidget.h"

class QStandardItemModel;
class QQuickWidget;

namespace Media
{
	class IEventsProvider;

	struct EventInfo;
	using EventInfos_t = QList<EventInfo>;
}

namespace LC::LMP
{
	class EventsWidget : public QWidget
	{
		Q_OBJECT

		Ui::EventsWidget Ui_;
		QQuickWidget * const View_;

		QStandardItemModel * const Model_;

		QList<Media::IEventsProvider*> Providers_;
	public:
		explicit EventsWidget (QWidget* = nullptr);

		void InitializeProviders ();
	private:
		void RequestEvents (int);
		void HandleEvents (const Media::EventInfos_t&);
	private slots:
		void handleAttendSure (int);
		void handleAttendMaybe (int);
		void handleUnattend (int);
	};
}
