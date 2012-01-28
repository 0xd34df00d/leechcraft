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

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_CAPSMANAGER_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_CAPSMANAGER_H
#include <QObject>
#include <QXmppDiscoveryIq.h>

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	class ClientConnection;
	class CapsDatabase;

	class CapsManager : public QObject
	{
		Q_OBJECT

		ClientConnection *Connection_;
		CapsDatabase *DB_;
		QHash<QString, QString> Caps2String_;
	public:
		CapsManager (ClientConnection*);

		void FetchCaps (const QString&, const QByteArray&);
		QStringList GetRawCaps (const QByteArray&) const;
		QStringList GetCaps (const QByteArray&) const;
		QStringList GetCaps (const QStringList&) const;

		QList<QXmppDiscoveryIq::Identity> GetIdentities (const QByteArray&) const;
	public slots:
		void handleInfoReceived (const QXmppDiscoveryIq&);
		void handleItemsReceived (const QXmppDiscoveryIq&);
	};
}
}
}

#endif
