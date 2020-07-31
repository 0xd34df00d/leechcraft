/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011 Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_PLUGINS_ACETAMIDE_CLIENTCONNECTION_H
#define PLUGINS_AZOTH_PLUGINS_ACETAMIDE_CLIENTCONNECTION_H

#include <QObject>
#include <QHash>
#include <QAbstractSocket>
#include "core.h"
#include "ircaccount.h"

namespace LC
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
		~ClientConnection ();

		IrcAccount* GetAccount () const;
		QList<IrcServerHandler*> GetServerHandlers () const;
		QList<QObject*> GetCLEntries () const;

		bool IsServerExists (const ServerOptions&) const;
		void JoinServer (const ServerOptions&);
		void JoinChannel (const ServerOptions&, const ChannelOptions&);

		void SetBookmarks (const QList<IrcBookmark>&);
		QList<IrcBookmark> GetBookmarks () const;

		IrcServerHandler* GetIrcServerHandler (const ServerOptions&) const;
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
		void gotConsoleLog (const QByteArray&, IHaveConsole::PacketDirection, const QString&);
	};
};
};
};
#endif // PLUGINS_AZOTH_PLUGINS_ACETAMIDE_CLIENTCONNECTION_H
