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

#include "generaladaptor.h"
#include <QCoreApplication>
#include <QDBusMessage>
#include "core.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace DBusManager
		{
			GeneralAdaptor::GeneralAdaptor (General *parent)
			: QDBusAbstractAdaptor (parent)
			, General_ (parent)
			{
			}

			QString GeneralAdaptor::GetOrganizationName () const
			{
				return QCoreApplication::organizationName ();
			}

			QString GeneralAdaptor::GetApplicationName () const
			{
				return QCoreApplication::applicationName ();
			}

			QStringList GeneralAdaptor::GetLoadedPlugins ()
			{
				return General_->GetLoadedPlugins ();
			}

			QString GeneralAdaptor::GetDescription (const QString& name,
					const QDBusMessage& msg)
			{
				try
				{
					return General_->GetDescription (name);
				}
				catch (const QString& str)
				{
					QDBusConnection::sessionBus ()
						.send (msg.createErrorReply ("GetDescription() failure",
									str));
					return str;
				}
			}

			QByteArray GeneralAdaptor::GetIcon (const QString& name, int dim,
					const QDBusMessage& msg)
			{
				try
				{
					return General_->GetIcon (name, dim);
				}
				catch (const QString& str)
				{
					QDBusConnection::sessionBus ()
						.send (msg.createErrorReply ("GetDescription() failure",
									str));
					return str.toUtf8 ();
				}
			}
		};
	};
};

