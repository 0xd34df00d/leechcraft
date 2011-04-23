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

#ifndef PLUGINS_AZOTH_PLUGINS_ACETAMIDE_CLIENTCONNECTION_H
#define PLUGINS_AZOTH_PLUGINS_ACETAMIDE_CLIENTCONNECTION_H

#include <QObject>
#include <QHash>
#include "core.h"
#include "ircaccount.h"

namespace LeechCraft
{

struct Entity;

namespace Azoth
{

class IProxyObject;

namespace Acetamide
{

	class ChannelCLEntry;
	class IrcServerHandler;
	class IrcServerCLEntry;

	class ClientConnection : public QObject
	{
		Q_OBJECT

		QString ChID_;
		IrcAccount *Account_;
		IProxyObject *ProxyObject_;
		QHash<QString, IrcServerHandler*> ServerHandlers_;
	public:
		ClientConnection (IrcAccount*);
		QObject* GetCLEntry (const QString&, const QString&) const;
		QList<QObject*> GetCLEntries () const;
		void Sinchronize ();

		IrcAccount* GetAccount () const;

		bool IsServerExists (const QString&);
		IrcServerCLEntry* JoinServer (const ServerOptions&);
		void JoinChannel (const ServerOptions&,
				const ChannelOptions&);
		IrcServerHandler* GetIrcServerHandler (const QString&);
		void ClosePrivateChat (QString, const QString&);
		void CloseServer (const QString&);
	signals:
		void gotRosterItems (const QList<QObject*>&);
		void rosterItemRemoved (QObject*);
		void rosterItemsRemoved (const QList<QObject*>&);
		void gotCLItems (const QList<QObject*>&);
	};
};
};
};
#endif // PLUGINS_AZOTH_PLUGINS_ACETAMIDE_CLIENTCONNECTION_H
