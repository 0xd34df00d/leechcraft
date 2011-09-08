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
	class IrcServerSocket;
	class UserCommandManager;
	class ServerResponceManager;
	class RplISupportParser;

	class IrcServerHandler : public QObject
	{
		Q_OBJECT

		IrcAccount *Account_;
		IrcErrorHandler *ErrorHandler_;
		IrcParser *IrcParser_;
		IrcServerCLEntry *ServerCLEntry_;
		IrcServerSocket *Socket_;
		UserCommandManager *CmdManager_;
		ServerResponceManager *ServerResponceManager_;
		RplISupportParser *RplISupportParser_;
		ConnectionState ServerConnectionState_;
		bool IsConsoleEnabled_;
		bool IsInviteDialogActive_;
		bool IsLongMessageInProcess_;
		QString ServerID_;
		QString NickName_;
		QString OldNickName_;
		QString LastSendId_;
		ServerOptions ServerOptions_;
		QList<ChannelOptions> ChannelsQueue_;
		std::auto_ptr<InviteChannelsDialog> InviteChannelsDialog_;
		QHash<QString, ChannelHandler*> ChannelHandlers_;
		QHash<QString, ServerParticipantEntry_ptr> Nick2Entry_;
		QMap<QString, QString> ISupport_;
	public:
		IrcServerHandler (const ServerOptions&,
				IrcAccount*);

		IrcServerCLEntry* GetCLEntry () const;
		IrcAccount* GetAccount () const;
		IrcParser* GetParser () const;
		QString GetNickName () const;
		QString GetServerID_ () const;
		ServerOptions GetServerOptions () const;
		QList<QObject*> GetCLEntries () const;
		QStringList GetPrivateChats () const;
		ChannelHandler* GetChannelHandler (const QString&);
		QList<ServerParticipantEntry_ptr> GetParticipantsOnChannel (const QString&);
		QList<ChannelHandler*> GetChannelHandlers () const;

		IrcMessage* CreateMessage (IMessage::MessageType,
				const QString&, const QString&);

		bool IsChannelExists (const QString&);
		bool IsParticipantExists (const QString&);

		void SetLongMessageState (bool);
		bool IsLongMessageInProcess () const;
		void SetNickName (const QString&);
		void Add2ChannelsQueue (const ChannelOptions&);

		void JoinChannel (const ChannelOptions&);
		bool JoinedChannel (const ChannelOptions&);
		void JoinChannelByCmd (const QStringList&);
		void JoinParticipant (const QString&, const QString&);

		void CloseChannel (const QString&);
		void LeaveParticipant (const QString&, const QString&, const QString&);
		void QuitServer ();
		void QuitParticipant (const QString&, const QString&);

		void SendMessage (const QStringList&);
		void IncomingMessage (const QString&, const QString&, const QString&);
		void IncomingNoticeMessage (const QString&, const QString&);

		void ChangeNickname (const QString&, const QString&);

		void GetBanList (const QString&);
		void GetExceptList (const QString&);
		void GetInviteList (const QString&);
		void AddBanListItem (const QString&, QString);
		void RemoveBanListItem (const QString&, QString);
		void AddExceptListItem (const QString&, QString);
		void RemoveExceptListItem (const QString&, QString);
		void AddInviteListItem (const QString&, QString);
		void RemoveInviteListItem (const QString&, QString);
		void SetNewChannelModes (const QString&, const ChannelModes&);

		void PongMessage (const QString&);
		void GotTopic (const QString&, const QString&);
		void KickUserFromChannel (const QString&, const QString&, 
				const QString&, const QString&);
		void GotInvitation (const QString&, const QString&);
		void ShowAnswer (const QString&);
		void CTCPReply (const QString&, const QString&, const QString&);
		void CTCPRequestResult (const QString&);
		void GotNames (const QString&, const QStringList&);
		void GotEndOfNames (const QString&);
		void ShowUserHost (const QString&, const QString&);
		void ShowIsUserOnServer (const QString&);
		void ShowWhoIsReply (const QString&);
		void ShowWhoWasReply (const QString&);
		void ShowWhoReply (const QString&);
		void ShowLinksReply (const QString&);
		void ShowInfoReply (const QString&);
		void ShowMotdReply (const QString&);
		void ShowUsersReply (const QString&);
		void ShowTraceReply (const QString&);
		void ShowStatsReply (const QString&);
		void ShowBanList (const QString&, 
				const QString&, const QString&, const QDateTime&);
		void ShowBanListEnd (const QString&);
		void ShowExceptList (const QString&, 
				const QString&, const QString&, const QDateTime&);
		void ShowExceptListEnd (const QString&);
		void ShowInviteList (const QString&, 
				const QString&, const QString&, const QDateTime&);
		void ShowInviteListEnd (const QString&);
		
		void SendPublicMessage (const QString&, const QString&);
		void SendPrivateMessage (IrcMessage*);
		void SendMessage2Server (const QStringList&);
		void ParseMessageForCommand (const QString&, const QString&);
		void LeaveChannel (const QString&, const QString&);
		void ClosePrivateChat (const QString&);
		void ConnectToServer ();
		void DisconnectFromServer ();
		void SendCommand (const QString&);
		ServerParticipantEntry_ptr GetParticipantEntry (const QString&);
		void RemoveParticipantEntry (const QString&);
		void UnregisterChannel (ChannelHandler*);
		void SetConsoleEnabled (bool);
		void LeaveAllChannel ();
		void CloseAllPrivateChats ();
		void SetLastSendID (const QString&);
		void ReadReply (const QByteArray&);
		void JoinFromQueue ();

		void SayCommand (const QStringList&);

		void ParseChanMode (const QString&, const QString&, 
				const QString& value = QString ());
		void ParseUserMode (const QString&, const QString&);

		void ParserISupport (const QString&);
		QMap<QString, QString> GetISupport () const;
	private:
		void SendToConsole (IMessage::Direction, const QString&);
		void NickCmdError ();
		ServerParticipantEntry_ptr CreateParticipantEntry (const QString&);
	private slots:
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
