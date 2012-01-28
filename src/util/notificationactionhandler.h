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

#ifndef UTIL_NOTIFICATIONACTIONHANDLER_H
#define UTIL_NOTIFICATIONACTIONHANDLER_H
#include <functional>
#include <QObject>
#include <interfaces/structures.h>
#include "utilconfig.h"

namespace LeechCraft
{
namespace Util
{
	class NotificationActionHandler : public QObject
	{
		Q_OBJECT

		Entity& Entity_;
	public:
		typedef std::function<void ()> Callback_t;
	private:
		QList<QPair<QString, Callback_t>> ActionName2Callback_;

		QList<QPointer<QObject>> DependentObjects_;
	public:
		UTIL_API NotificationActionHandler (Entity&, QObject* = 0);

		UTIL_API void AddFunction (const QString&, Callback_t);
		UTIL_API void AddDependentObject (QObject*);
	public slots:
		void notificationActionTriggered (int);
	};
}
}

#endif
