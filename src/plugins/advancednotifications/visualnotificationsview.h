/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QtGlobal>
#include <QQuickWidget>
#include <interfaces/core/icoreproxyfwd.h>
#include "eventdata.h"

namespace LC
{
namespace AdvancedNotifications
{
	class VisualNotificationsView : public QQuickWidget
	{
		Q_OBJECT

		QObjectList LastEvents_;
		QUrl Location_;
	public:
		VisualNotificationsView (const ICoreProxy_ptr&);

		void SetEvents (const QList<EventData>&);
	signals:
		void actionTriggered (const QString&, int);
		void dismissEvent (const QString&);
	};
}
}
