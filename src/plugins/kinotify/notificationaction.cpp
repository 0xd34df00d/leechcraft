/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Oleg Linkin
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

#include "notificationaction.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Kinotify
		{
			NotificationAction::NotificationAction (QObject *parent) 
			: QObject (parent)
			{
			}
			
			void NotificationAction::SetActionObject (QObject* obj)
			{
				ActionObject_ = obj;
			}
			
			void NotificationAction::sendActionOnClick (const QString& idx)
			{
				QMetaObject::invokeMethod (ActionObject_,
						"notificationActionTriggered",
						Qt::QueuedConnection,
						Q_ARG (int, idx.toInt ()));
				
				emit actionPressed ();
			}
		};
	};
};
