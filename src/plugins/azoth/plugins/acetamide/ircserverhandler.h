/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
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
#include "invitechannelsdialog.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	class ChannelCLEntry;
	class ChannelHandler;
	class IrcAccount;
	class IrcErrorHandler;
	class IrcMessage;
	class IrcParser;
	class IrcServerCLEntry;

	class IrcServerHandler : public QObject
	{
		Q_OBJECT

		IrcAccount *Account_;
		IrcErrorHandler *ErrorHandler_;
		IrcParser *IrcParser_;
		IrcServerCLEntry *ServerCLEntry_;
		ConnectionState ServerConnectionState_;
		bool IsConsoleEnabled_;
		bool IsInviteDialogActive_;
		QString ServerID_;
		QString NickName_;
		QString OldNickName_;
		QString LastSendId_;
		QVariantMap ISupport_;
		ServerOptions ServerOptions_;
		QList<ChannelOptions> ChannelsQueue_;
		std::auto_ptr<InviteChannelsDialog> InviteChannelsDialog_;
		boost::shared_ptr<QTcpSocket> TcpSocket_ptr;
		QHash<QString, ChannelHandler*> ChannelHandlers_;
		QHash<QString, boost::function<void (const QString&,
				const QList<std::string>&, const QString&)> > Command2Action_;
		QHash<QString, boost::function<void (const QStringList&)> > Name2Command_;
		QHash<QString, ServerParticipantEntry_ptr> Nick2Entry_;
	public:
		IrcServerHandler (const ServerOptions&, IrcAccount*);
		IrcServerCLEntry* GetCLEntry () const;
		IrcAccount* GetAccount () const;
		IrcParser* GetParser () const;
		QString GetNickName () const;
		void SetNickName (const QString&);
		QString GetServerID_ () const;
		ServerOptions GetServerOptions () const;
		bool IsChannelExists (const QString&);
		bool IsParticipantExists (const QString&);
		void Add2ChannelsQueue (const ChannelOptions&);
		void SendPublicMessage (const QString&, const QString&);
		void SendPrivateMessage (IrcMessage*);
		void SendCommandMessage2Server (const QString&);
		void ParseMessageForCommand (const QString&, const QString&);
		QList<QObject*> GetCLEntries () const;
		void LeaveChannel (const QString&, const QString&);
		QStringList GetPrivateChats () const;
		void ClosePrivateChat (const QString&);
		ChannelHandler* GetChannelHandler (const QString&);
		QList<ServerParticipantEntry_ptr> GetParticipants (const QString&);
		IrcMessage* CreateMessage (IMessage::MessageType,
				const QString&, const QString&);
		void ConnectToServer ();
		void DisconnectFromServer ();
		bool JoinChannel (const ChannelOptions&);
		void JoinChannelByCmd (const QStringList&);
		void SendCommand (const QString&);
		void IncomingMessage2Server ();
		void IncomingMessage2Channel ();
		void IncomingMessage2Channel (const QString&);
		ServerParticipantEntry_ptr GetParticipantEntry (const QString&);
		void RemoveParticipantEntry (const QString&);
		void UnregisterChannel (ChannelHandler*);
		void SetConsoleEnabled (bool);
		void LeaveAllChannel ();
		void CloseAllPrivateChats ();
	private:
		void SendToConsole (IMessage::Direction, const QString&);
		void InitCommandResponses ();
		void InitSocket ();
		bool IsCTCPMessage (const QString&);
		void NickCmdError ();
		QString EncodedMessage (const QString&, IMessage::Direction);
		ServerParticipantEntry_ptr
				CreateParticipantEntry (const QString&);
		void JoinFromQueue ();
		void ShowAnswer (const QString&);
		// RPL
		void SetTopic (const QString&,
				const QList<std::string>&, const QString&);
		void AddParticipants (const QString&,
				const QList<std::string>&, const QString&);
		void JoinParticipant (const QString&,
				const QList<std::string>&, const QString&);
		void LeaveParticipant (const QString&,
				const QList<std::string>&, const QString&);
		void QuitParticipant (const QString&,
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
		void GetWhoIsChannels (const QString&,
				const QList<std::string>&, const QString&);
		void GetWhoWas (const QString&,
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
		void GetLinks (const QString&,
				const QList<std::string>&, const QString&);
		void GetInfo (const QString&,
				const QList<std::string>&, const QString&);
		void GetMotd (const QString&,
				const QList<std::string>&, const QString&);
		void GetEndMessage (const QString&,
				const QList<std::string>&, const QString&);
		void GetYoureOper (const QString&,
				const QList<std::string>&, const QString&);
		void GetRehash (const QString&,
				const QList<std::string>&, const QString&);
		void GetTime (const QString&,
				const QList<std::string>&, const QString&);
		void GetUsersStart (const QString&,
				const QList<std::string>&, const QString&);
		void GetUsers (const QString&,
				const QList<std::string>&, const QString&);
		void GetNoUser (const QString&,
				const QList<std::string>&, const QString&);
		void GetTraceLink (const QString&,
				const QList<std::string>&, const QString&);
		void GetTraceConnecting (const QString&,
				const QList<std::string>&, const QString&);
		void GetTraceHandshake (const QString&,
				const QList<std::string>&, const QString&);
		void GetTraceUnknown (const QString&,
				const QList<std::string>&, const QString&);
		void GetTraceOperator (const QString&,
				const QList<std::string>&, const QString&);
		void GetTraceUser (const QString&,
				const QList<std::string>&, const QString&);
		void GetTraceServer (const QString&,
				const QList<std::string>&, const QString&);
		void GetTraceService (const QString&,
				const QList<std::string>&, const QString&);
		void GetTraceNewType (const QString&,
				const QList<std::string>&, const QString&);
		void GetTraceClass (const QString&,
				const QList<std::string>&, const QString&);
		void GetTraceLog (const QString&,
				const QList<std::string>&, const QString&);
		void GetTraceEnd (const QString&,
				const QList<std::string>&, const QString&);
		void GetStatsLinkInfo (const QString&,
				const QList<std::string>&, const QString&);
		void GetStatsCommands (const QString&,
				const QList<std::string>&, const QString&);
		void GetStatsEnd (const QString&,
				const QList<std::string>&, const QString&);
		void GetStatsUptime (const QString&,
				const QList<std::string>&, const QString&);
		void GetStatsOline (const QString&,
				const QList<std::string>&, const QString&);
		void GetLuserClient (const QString&,
				const QList<std::string>&, const QString&);
		void GetLuserOp (const QString&,
				const QList<std::string>&, const QString&);
		void GetLuserUnknown (const QString&,
				const QList<std::string>&, const QString&);
		void GetLuserChannels (const QString&,
				const QList<std::string>&, const QString&);
		void GetLuserMe (const QString&,
				const QList<std::string>&, const QString&);
		void GetAdmineMe (const QString&,
				const QList<std::string>&, const QString&);
		void GetAdminLoc1 (const QString&,
				const QList<std::string>&, const QString&);
		void GetAdminLoc2 (const QString&,
				const QList<std::string>&, const QString&);
		void GetAdminEmail (const QString&,
				const QList<std::string>&, const QString&);
		void GetTryAgain (const QString&,
				const QList<std::string>&, const QString&);
	private slots:
		void readReply ();
		void connectionEstablished ();
		void connectionClosed ();
		void joinAfterInvite ();
	signals:
		void connected (const QString&);
		void disconnected (const QString&);
		void sendMessageToConsole (IMessage::Direction, const QString&);
		void nicknameConflict (const QString&);
	};
};
};
};
#endif // PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCSERVERHANDLER_H
