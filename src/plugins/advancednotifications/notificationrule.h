/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#ifndef PLUGINS_ADVANCEDNOTIFICATIONS_NOTIFICATIONRULE_H
#define PLUGINS_ADVANCEDNOTIFICATIONS_NOTIFICATIONRULE_H
#include <QStringList>
#include <QMetaType>
#include "common.h"

class QDataStream;

namespace LeechCraft
{
namespace AdvancedNotifications
{
	class NotificationRule
	{
		QString Name_;
		QString Category_;
		QStringList Types_;
		
		NotificationMethods Methods_;
	public:
		NotificationRule ();
		NotificationRule (const QString& name,
				const QString& cat, const QStringList& types);
		
		QString GetName () const;
		void SetName (const QString&);
		
		QString GetCategory () const;
		void SetCategory (const QString&);
		
		QStringList GetTypes () const;
		void SetTypes (const QStringList&);
		
		NotificationMethods GetMethods () const;
		void SetMethods (const NotificationMethods&);
		
		void Save (QDataStream&) const;
		void Load (QDataStream&);
	};
}
}

Q_DECLARE_METATYPE (LeechCraft::AdvancedNotifications::NotificationRule);
Q_DECLARE_METATYPE (QList<LeechCraft::AdvancedNotifications::NotificationRule>);
	
#endif
