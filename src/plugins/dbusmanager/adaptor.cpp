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

#include "adaptor.h"
#include <QCoreApplication>
#include "core.h"

using namespace LeechCraft::Plugins::DBusManager;

Adaptor::Adaptor (Core *parent)
: QDBusAbstractAdaptor (parent)
, Core_ (parent)
{
	connect (parent,
			SIGNAL (aboutToQuit ()),
			this,
			SIGNAL (aboutToQuit ()));
	connect (parent,
			SIGNAL (someEventHappened (const QString&)),
			this,
			SIGNAL (someEventHappened (const QString&)));
}

QString Adaptor::GetOrganizationName () const
{
	return QCoreApplication::organizationName ();
}

QString Adaptor::GetApplicationName () const
{
	return QCoreApplication::applicationName ();
}

QString Adaptor::Greeter (const QString& msg,
		const QDBusMessage&)
{
	return Core_->Greeter (msg);
}

QStringList Adaptor::GetLoadedPlugins ()
{
	return Core_->GetLoadedPlugins ();
}

