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
#include "ircserverconsole.h"
#include "localtypes.h"
#include "serverparticipantentry.h"
#include "invitechannelsdialog.h"

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
		IrcServerConsole_ptr Console_;
		bool IsConsoleEnabled_;
		bool ChannelJoined_;
		bool IsInviteDialogActive_;
		std::auto_ptr<InviteChannelsDialog> InviteChannelsDialog_;

		ServerOptions ServerOptions_;
		QString ServerID_;
		boost::shared_ptr<QTcpSocket> TcpSocket_ptr;
		ConnectionState ServerConnectionState_;
		QHash<QString, ChannelHandler*> ChannelHandlers_;
		QHash<QString, boost::function<void (void)> > Error2Action_;
		QHash<QString, boost::function<void (const QString&,
				QList<std::string>, const QString&)> > Command2Action_;
		QHash<QString, boost::function<void (const QStringList&)> >
				Name2Command_;
		QHash<QString, ServerParticipantEntry_ptr> Nick2Entry_;
		QString NickName_;
		QString OldNickName_;
		QList<ChannelOptions> ChannelsQueue_;
		QVariantMap ISupport_;
	public:
		IrcServerHandler (const ServerOptions&, IrcAccount*);
		IrcServerCLEntry* GetCLEntry () const;
		IrcAccount* GetAccount () const;
		QString GetNickName () const;

		IrcServerConsole_ptr GetIrcServerConsole () const;

		QString GetServerID_ () const;
		ServerOptions GetServerOptions () const;
		ConnectionState GetConnectionState () const;
		bool IsChannelExists (const QString&);
		bool IsParticipantExists (const QString&);

		void Add2ChannelsQueue (const ChannelOptions&);

		void SendPublicMessage (const QString&, const QString&);
		void SendPrivateMessage (IrcMessage*);
		void ParseMessageForCommand (const QString&, const QString&);

		QList<QObject*> GetCLEntries () const;

		void LeaveChannel (const QString&, const QString&);
		QStringList GetPrivateChats () const;
		void ClosePrivateChat (const QString&);

		ChannelHandler* GetChannelHandler (const QString&);
		QList<ChannelHandler*> GetChannelHandlers () const;
		QList<ServerParticipantEntry_ptr>
				GetParticipants (const QString&);

		bool IsRoleAvailable (ChannelRole);

		IrcMessage* CreateMessage (IMessage::MessageType,
				const QString&, const QString&);

		void ConnectToServer ();
		bool DisconnectFromServer ();
		bool JoinChannel (const ChannelOptions&);
		void JoinChannelByCmd (const QStringList&);
		void SendCommand (const QString&);
		void IncomingMessage2Server ();
		void IncomingMessage2Channel ();
		ServerParticipantEntry_ptr GetParticipantEntry (const QString&);
		void RemoveParticipantEntry (const QString&);

		void UnregisterChannel (ChannelHandler*);
	private:
		void SendToConsole (const QString&);
		void InitErrorsReplys ();
		void InitCommandResponses ();
		void InitSocket ();
		bool IsErrorReply (const QString&);
		bool IsCTCPMessage (const QString&);

		void NoSuchNickError ();
		void NickCmdError ();

		void SendAnswerToChannel (const QString&, const QString&,
				bool remove = false);

		QString EncodedMessage (const QString&, IMessage::Direction);

		ServerParticipantEntry_ptr
				CreateParticipantEntry (const QString&);

		// RPL
		void JoinFromQueue (const QString&,
				const QList<std::string>&, const QString&);
		void SetTopic (const QString&,
				const QList<std::string>&, const QString&);
		void AddParticipants (const QString&,
				const QList<std::string>&, const QString&);
		void JoinParticipant (const QString&,
				const QList<std::string>&, const QString&);
		void LeaveParticipant (const QString&,
				const QList<std::string>&, const QString&);
		void HandleIncomingMessage (const QString&,
				const QList<std::string>&, const QString&);
		void PongMessage (const QString&,
				const QList<std::string>&, const QString&);
		void SetISupport (const QString&,
				const QList<std::string>&, const QString&);
		void ChangeNickname (const QString&,
				const QList<std::string>&, const QString&);
		void CTCPReply (const QString&,
				const QList<std::string>&, const QString&);
		void CTCPRequestResult (const QString&,
				const QList<std::string>&, const QString&);
		void InviteToChannel (const QString&,
				const QList<std::string>&, const QString&);
		void KickFromChannel (const QString&,
				const QList<std::string>&, const QString&);
		void GetUserHost (const QString&,
				const QList<std::string>&, const QString&);
		void GetIson (const QString&,
				const QList<std::string>&, const QString&);
		void GetAway (const QString&,
				const QList<std::string>&, const QString&);
		void GetWhoIsUser (const QString&,
				const QList<std::string>&, const QString&);
		void GetWhoIsServer (const QString&,
				const QList<std::string>&, const QString&);
		void GetWhoIsOperator (const QString&,
				const QList<std::string>&, const QString&);
		void GetWhoIsIdle (const QString&,
				const QList<std::string>&, const QString&);
		void GetWhoIsEnd (const QString&,
				const QList<std::string>&, const QString&);
		void GetWhoIsChannels (const QString&,
				const QList<std::string>&, const QString&);
		void GetWhoWas (const QString&,
				const QList<std::string>&, const QString&);
		void GetWhoWasEnd (const QString&,
				const QList<std::string>&, const QString&);
		void GetNoTopic (const QString&,
				const QList<std::string>&, const QString&);
		void GetInviting (const QString&,
				const QList<std::string>&, const QString&);
		void GetSummoning (const QString&,
				const QList<std::string>&, const QString&);
		void GetVersion (const QString&,
				const QList<std::string>&, const QString&);
		void GetWho (const QString&,
				const QList<std::string>&, const QString&);
		void GetWhoEnd (const QString&,
				const QList<std::string>&, const QString&);
		void GetNamesEnd (const QString&,
				const QList<std::string>&, const QString&);
		void GetLinks (const QString&,
				const QList<std::string>&, const QString&);
		void GetLinksEnd (const QString&,
				const QList<std::string>&, const QString&);
	private slots:
		void readReply ();
		void connectionEstablished ();
		void joinAfterInvite ();
	signals:
		void connected (const QString&);
	};
};
};
};
#endif // PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCSERVERHANDLER_H
