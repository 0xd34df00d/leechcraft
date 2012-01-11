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

#ifndef PLUGINS_ADVANCEDNOTIFICATIONS_QML_EVENTPROXYOBJECT_H
#define PLUGINS_ADVANCEDNOTIFICATIONS_QML_EVENTPROXYOBJECT_H
#include <QObject>
#include <QUrl>
#include "../eventdata.h"

namespace LeechCraft
{
namespace AdvancedNotifications
{
	class EventProxyObject : public QObject
	{
		Q_OBJECT
		Q_PROPERTY (int count READ count NOTIFY countChanged);
		Q_PROPERTY (QUrl image READ image NOTIFY imageChanged);
		Q_PROPERTY (QString extendedText READ extendedText NOTIFY extendedTextChanged);
		Q_PROPERTY (QVariant eventActionsModel READ eventActionsModel NOTIFY eventActionsModelChanged);
		
		EventData E_;
		QUrl CachedImage_;
		QVariant ActionsModel_;
	public:
		EventProxyObject (const EventData&, QObject* = 0);
		
		int count () const;
		QUrl image () const;
		QString extendedText () const;
		
		QVariant eventActionsModel () const;
	private slots:
		void handleActionSelected ();
		void handleDismissEvent ();
	signals:
		void countChanged ();
		void imageChanged ();
		void extendedTextChanged ();
		
		void eventActionsModelChanged ();
		
		void dismissEvent ();
		
		void actionTriggered (const QString&, int);
		void dismissEventRequested (const QString&);
	};
}
}

#endif
