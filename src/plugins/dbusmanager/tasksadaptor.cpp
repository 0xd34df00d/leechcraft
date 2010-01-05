/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#include "tasksadaptor.h"
#include <QCoreApplication>
#include <QDBusMessage>
#include <QDBusArgument>
#include <QDBusMetaType>
#include "core.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace DBusManager
		{
			TasksAdaptor::TasksAdaptor (Tasks *parent)
			: QDBusAbstractAdaptor (parent)
			, Tasks_ (parent)
			{
				qDBusRegisterMetaType<QVariantList> ();
			}

			QStringList TasksAdaptor::GetHolders () const
			{
				return Tasks_->GetHolders ();
			}

			int TasksAdaptor::RowCount (const QString& name,
					const QDBusMessage& msg) const
			{
				try
				{
					return Tasks_->RowCount (name);
				}
				catch (const QString& str)
				{
					QDBusConnection::sessionBus ()
						.send (msg.createErrorReply ("RowCount() failure",
									str));
					return -1;
				}
			}

			QVariantList TasksAdaptor::GetData (const QString& name,
					int r, int role,
					const QDBusMessage& msg) const
			{
				try
				{
					return Tasks_->GetData (name, r, role);
				}
				catch (const QString& str)
				{
					QDBusConnection::sessionBus ()
						.send (msg.createErrorReply ("GetData() failure",
									str));
					return QVariantList () << str;
				}
			}
		};
	};
};

