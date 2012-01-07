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

#ifndef PLUGINS_ADVANCEDNOTIFICATIONS_QML_VISUALNOTIFICATIONSVIEW_H
#define PLUGINS_ADVANCEDNOTIFICATIONS_QML_VISUALNOTIFICATIONSVIEW_H

#ifndef HAVE_QML
#error "Trying to compile QML notifications view without having QML :("
#endif

#include <QDeclarativeView>
#include "../eventdata.h"

namespace LeechCraft
{
namespace AdvancedNotifications
{
	class VisualNotificationsView : public QDeclarativeView
	{
		Q_OBJECT
		
		QObjectList LastEvents_;
		QUrl Location_;
	public:
		VisualNotificationsView (QWidget* = 0);
		
		void SetEvents (const QList<EventData>&);
	private slots:
		void handleStatusChanged (QDeclarativeView::Status);
	signals:
		void actionTriggered (const QString&, int);
		void dismissEvent (const QString&);
	};
}
}

#endif
