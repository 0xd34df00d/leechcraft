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
#include "ircaccount.h"
#include "ircserver.h"
#include "core.h"

namespace LeechCraft
{
struct Entity;
	
namespace Azoth
{
class IProxyObject;

namespace Acetamide
{

	class ChannelCLEntry;
	class ChannelHandler;
	class IrcMessage;

	class ClientConnection : public QObject
	{
		Q_OBJECT
		
		QString ChID_;
		IrcAccount *Account_;
		IProxyObject *ProxyObject_;

		QHash<QString, ChannelHandler*> ChannelHandlers_;
		QHash<QString, IrcServer_ptr> IrcServers_;
		QHash<QString, QHash<QString, ServerParticipantEntry_ptr> > Server2Entry_;
	public:
		ClientConnection (IrcAccount*);
		virtual ~ClientConnection ();
		QObject* GetCLEntry (const QString&, const QString&) const;
		QList<QObject*> GetCLEntries () const;
		void Sinchronize ();

		QList<QObject*> GetChannelCLEntries (const QString&) const;
		IrcAccount* GetAccount () const;
		ChannelCLEntry* JoinRoom (const ServerOptions&, const ChannelOptions&);
		void Unregister (ChannelHandler*);
		IrcMessage* CreateMessage (IMessage::MessageType,
				const QString&, const QString&);
		void SetNewParticipant (const QString&, const QString&);
		void SetUserLeave (const QString&, const QString&, const QString&);
		void SetPrivateMessage (IrcAccount*, IrcMessage*);
		ServerParticipantEntry_ptr GetServerParticipantEntry (const QString&, 
				const QString&, bool announce = true);
	private:
		ServerParticipantEntry_ptr CreateServerParticipantEntry (const QString&, 
				const QString&, bool);
	public slots:
		void setChannelUseres (const QString&, const QString&);
		void setSubject (const QString&, const QString&);
		void handleMessageReceived (const QString&, const QString&, const QString&);
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
