/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Georg Rudoy
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

#ifndef PLUGINS_AZOTH_NOTIFICATIONACTIONHANDLER_H
#define PLUGINS_AZOTH_NOTIFICATIONACTIONHANDLER_H
#include <boost/function.hpp>
#include <QObject>
#include <interfaces/structures.h>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Azoth
		{
			class NotificationActionHandler : public QObject
			{
				Q_OBJECT

				Entity& Entity_;
			public:
				typedef boost::function<void ()> Callback_t;
			private:
				QList<QPair<QString, Callback_t> > ActionName2Callback_;
			public:
				NotificationActionHandler (Entity&, QObject* = 0);

				void AddFunction (const QString&, Callback_t);
			public slots:
				void notificationActionTriggered (int);
			};
		}
	}
}

#endif
