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

#include "protocol.h"
#include <QIcon>
#include <QtDebug>

namespace LeechCraft
{
namespace Azoth
{
namespace VelvetBird
{
	Protocol::Protocol (PurplePlugin *plug, ICoreProxy_ptr proxy, QObject *parent)
	: QObject (parent)
	, Proxy_ (proxy)
	, PPlug_ (plug)
	{
	}

	QObject* Protocol::GetObject()
	{
		return this;
	}

	IProtocol::ProtocolFeatures Protocol::GetFeatures () const
	{
		return PFNone;
	}

	QList<QObject*> Protocol::GetRegisteredAccounts ()
	{
		return {};
	}

	QObject* Protocol::GetParentProtocolPlugin () const
	{
		return parent ();
	}

	QString Protocol::GetProtocolName () const
	{
		return QString::fromUtf8 (purple_plugin_get_name (PPlug_));
	}

	QIcon Protocol::GetProtocolIcon () const
	{
		QByteArray id (purple_plugin_get_id (PPlug_));
		if (id.startsWith ("prpl-"))
			id.remove (0, 5);

		QIcon result = QIcon::fromTheme (QString::fromUtf8 ("im-" + id));
		if (result.isNull ())
			result = QIcon (":/azoth/velvetbird/resources/images/velvetbird.svg");
		return result;
	}

	QByteArray Protocol::GetProtocolID () const
	{
		return QByteArray ("VelvetBird.") + purple_plugin_get_id (PPlug_);
	}

	QList<QWidget*> Protocol::GetAccountRegistrationWidgets (IProtocol::AccountAddOptions opts)
	{
		return {};
	}

	void Protocol::RegisterAccount (const QString& name, const QList<QWidget*>& widgets)
	{
	}

	QWidget* Protocol::GetMUCJoinWidget ()
	{
		return 0;
	}

	void Protocol::RemoveAccount (QObject *account)
	{
	}
}
}
}
