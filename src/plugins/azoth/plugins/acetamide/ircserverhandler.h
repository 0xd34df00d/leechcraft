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

#ifndef PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCSERVERHANDLER_H
#define PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCSERVERHANDLER_H

#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <QObject>
#include <QTcpSocket>
#include <interfaces/imessage.h>
#include "localtypes.h"
#include "serverparticipantentry.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{

class IrcMessage;

	class ChannelCLEntry;
	class ChannelHandler;
	class IrcAccount;
	class IrcParser;
	class IrcServerCLEntry;

	class IrcServerHandler : public QObject
	{
		Q_OBJECT

		IrcAccount *Account_;
		IrcParser *IrcParser_;
		IrcServerCLEntry *ServerCLEntry_;
		ServerOptions ServerOptions_;
		QString ServerID_;
		boost::shared_ptr<QTcpSocket> TcpSocket_ptr;
		ConnectionState ServerConnectionState_;
		QHash<QString, ChannelHandler*> ChannelHandlers_;
		QHash<QString,
				boost::function<void (void)> > Error2Action_;
		QHash<QString,
				boost::function<void (const QString&,
					QList<std::string>,
					const QString&)> > Command2Action_;
		QHash<QString, ServerParticipantEntry_ptr> Nick2Entry_;
		QString NickName_;
		QList<ChannelOptions> ChannelsQueue_;
	public:
		IrcServerHandler (const ServerOptions&, IrcAccount*);
		IrcServerCLEntry* GetCLEntry () const;
		IrcAccount* GetAccount () const;
		QString GetNickName () const;

		QString GetServerID_ () const;
		ServerOptions GetServerOptions () const;
		ConnectionState GetConnectionState () const;
		bool IsChannelExists (const QString&);

		void Add2ChannelsQueue (const ChannelOptions&);

		void SendPublicMessage (const QString&, const QString&);

		ChannelHandler* GetChannelHandler (const QString&);
		QList<ChannelHandler*> GetChannelHandlers () const;

		IrcMessage* CreateMessage (IMessage::MessageType,
				const QString&, const QString&);

		bool ConnectToServer ();
		bool JoinChannel (const ChannelOptions&);
		void SendCommand (const QString&);
		void IncomingMessage2Server ();
		void IncomingMessage2Channel ();
		ServerParticipantEntry_ptr GetParticipantEntry (const QString&);
		void RemoveParticipantEntry (const QString&);
	private:
		void InitErrorsReplys ();
		void InitCommandResponses ();
		void InitSocket ();
		bool IsErrorReply (const QString&);

		void NoSuchNickError ();
		void NickCmdError ();

		QString EncodedMessage (const QString&, IMessage::Direction);

		ServerParticipantEntry_ptr
				CreateParticipantEntry (const QString&);

		void JoinFromQueue (const QString&,
				QList<std::string>, const QString&);
		void SetTopic (const QString&,
				QList<std::string>, const QString&);
		void AddParticipants (const QString&,
				QList<std::string>, const QString&);
		void JoinParticipant (const QString&,
				QList<std::string>, const QString&);
		void LeaveParticipant (const QString&,
				QList<std::string>, const QString&);
		void HandleIncomingMessage (const QString&,
				QList<std::string>, const QString&);
	private slots:
		void readReply ();
	signals:
		void gotCLItems (const QList<QObject*>&);
	};
};
};
};
#endif // PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCSERVERHANDLER_H
