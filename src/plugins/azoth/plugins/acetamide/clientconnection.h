/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011 Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QHash>
#include "ircaccount.h"

namespace LC::Azoth::Acetamide
{
	class ChannelCLEntry;
	class IrcServerHandler;
	class IrcServerCLEntry;

	class ClientConnection : public QObject
	{
		Q_OBJECT

		IrcAccount *Account_;
		QHash<QString, IrcServerHandler*> ServerHandlers_;
		bool IsConsoleEnabled_ = false;
	public:
		explicit ClientConnection (IrcAccount*);
		~ClientConnection () override;

		IrcAccount* GetAccount () const;
		QList<IrcServerHandler*> GetServerHandlers () const;
		QList<QObject*> GetCLEntries () const;

		bool IsServerExists (const ServerOptions&) const;
		IrcServerHandler& JoinServer (const ServerOptions&);
		void JoinChannel (const ServerOptions&, const ChannelOptions&);

		void SetBookmarks (const QList<IrcBookmark>&);
		QList<IrcBookmark> GetBookmarks () const;

		IrcServerHandler* GetIrcServerHandler (const ServerOptions&) const;
		IrcServerHandler* GetIrcServerHandler (const QString&) const;

		void DisconnectFromAll ();
		void QuitServer (const QStringList&);

		void SetConsoleEnabled (bool);

		void ClosePrivateChat (const QString& serverID, const QString& nick);

		void FetchVCard (const QString& serverId, const QString& nick);

		void SetAway (bool away, const QString& message);

		QString GetStatusStringForState (Azoth::State state);
	public slots:
		void serverConnected (const QString&);
		void serverDisconnected (const QString&);
	private:
		void HandleLog (IMessage::Direction, const QString&);
	};
}
