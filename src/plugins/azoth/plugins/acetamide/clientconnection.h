/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011 Oleg Linkin
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
#include <QAbstractSocket>
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
		bool IsConsoleEnabled_;
	public:
		ClientConnection (IrcAccount*);
		void Sinchronize ();

		IrcAccount* GetAccount () const;
		QList<IrcServerHandler*> GetServerHandlers () const;

		bool IsServerExists (const QString&);
		void JoinServer (const ServerOptions&);
		void JoinChannel (const ServerOptions&, const ChannelOptions&);

		void SetBookmarks (const QList<IrcBookmark>&);
		QList<IrcBookmark> GetBookmarks () const;

		IrcServerHandler* GetIrcServerHandler (const QString&) const;

		void DisconnectFromAll ();
		void QuitServer (const QStringList&);

		void SetConsoleEnabled (bool);

		void ClosePrivateChat (const QString& serverID, QString nick);

		void FetchVCard (const QString& serverId, const QString& nick);

		void SetAway (bool away, const QString& message);

		QString GetStatusStringForState (Azoth::State state);
	public slots:
		void serverConnected (const QString&);
		void serverDisconnected (const QString&);
		void handleError (QAbstractSocket::SocketError error,
				const QString& errorString);
		void handleLog (IMessage::Direction, const QString&);
	signals:
		void gotRosterItems (const QList<QObject*>&);
		void rosterItemRemoved (QObject*);
		void rosterItemsRemoved (const QList<QObject*>&);
		void gotCLItems (const QList<QObject*>&);
		void gotConsoleLog (const QByteArray&, int);
	};
};
};
};
#endif // PLUGINS_AZOTH_PLUGINS_ACETAMIDE_CLIENTCONNECTION_H
