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

#pragma once

#include <QObject>
#include <QXmppDiscoveryIq.h>

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	class AccountSettingsHolder;
	class ClientConnection;

	class ServerInfoStorage : public QObject
	{
		Q_OBJECT

		ClientConnection *Conn_;
		AccountSettingsHolder *Settings_;
		QString PreviousJID_;
		QString Server_;

		QStringList ServerFeatures_;

		QString BytestreamsProxy_;
	public:
		ServerInfoStorage (ClientConnection*, AccountSettingsHolder*);

		bool HasServerFeatures () const;
		QString GetBytestreamsProxy () const;
	private:
		void HandleItems (const QXmppDiscoveryIq&);
		void HandleServerInfo (const QXmppDiscoveryIq&);
		void HandleItemInfo (const QXmppDiscoveryIq&);
	private slots:
		void handleConnected ();
	signals:
		void bytestreamsProxyChanged (const QString&);
	};
}
}
}
