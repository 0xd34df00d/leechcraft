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
	class ChannelsManager;

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
		ChannelsManager *ChannelsManager_;

		ConnectionState ServerConnectionState_;
		bool IsConsoleEnabled_;
		bool IsInviteDialogActive_;
		QString ServerID_;
		QString NickName_;
		QString OldNickName_;
		QString LastSendId_;
		ServerOptions ServerOptions_;
		std::unique_ptr<InviteChannelsDialog> InviteChannelsDialog_;
		QHash<QString, ServerParticipantEntry_ptr> Nick2Entry_;
		QMap<QString, QString> ISupport_;
	public:
		IrcServerHandler (const ServerOptions&,
				IrcAccount*);

		IrcServerCLEntry* GetCLEntry () const;
		IrcAccount* GetAccount () const;
		IrcParser* GetParser () const;
		ChannelsManager* GetChannelManager () const;
		QString GetNickName () const;
		QString GetServerID () const;
		ServerOptions GetServerOptions () const;
		QObjectList GetCLEntries () const;

		ChannelHandler* GetChannelHandler (const QString& channel);
		QList<ChannelHandler*> GetChannelHandlers () const;

		IrcMessage* CreateMessage (IMessage::MessageType type,
				const QString& variant, const QString& body);

		bool IsChannelExists (const QString& channel);

		void SetNickName (const QString& nick);
		void Add2ChannelsQueue (const ChannelOptions& options);

		void JoinChannel (const ChannelOptions& options);
		bool JoinedChannel (const ChannelOptions& options);
		void JoinParticipant (const QString& nick, const QString& msg,
				const QString& user = QString (), const QString& host = QString ());

		void CloseChannel (const QString& channel);
		void LeaveParticipant (const QString& nick,
				const QString& channel, const QString& msg);

		void QuitServer ();
		void QuitParticipant (const QString& nick, const QString& msg);

		void SendMessage (const QStringList&);
		void IncomingMessage (const QString&, const QString&, const QString&);
		void IncomingNoticeMessage (const QString&, const QString&);

		void ChangeNickname (const QString&, const QString&);

		bool IsCmdHasLongAnswer (const QString& cmd);

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
		void SetNewChannelMode (const QString&, const QString&, const QString&);

		void PongMessage (const QString&);
		void GotTopic (const QString&, const QString&);
		void GotKickCommand (const QString&, const QString&,
				const QString&, const QString&);
		void KickParticipant (const QString&, const QString&,
				const QString&);
		void GotInvitation (const QString&, const QString&);
		void ShowAnswer (const QString& cmd,
                const QString& answer, bool isEndOf = false);

		void CTCPReply (const QString&, const QString&, const QString&);
		void CTCPRequestResult (const QString&);
		void CTCPRequst (const QStringList& cmd);

		void GotNames (const QString&, const QStringList&);
		void GotEndOfNames (const QString&);

		void ShowUserHost (const QString&, const QString&);
		void ShowIsUserOnServer (const QString&);

		void ShowWhoIsReply (const QString&, bool isEndOf = false);
		void ShowWhoWasReply (const QString&, bool isEndOf = false);
		void ShowWhoReply (const QString&, bool isEndOf = false);
		void ShowLinksReply (const QString&, bool isEndOf = false);
		void ShowInfoReply (const QString&, bool isEndOf = false);
		void ShowMotdReply (const QString&, bool isEndOf = false);
		void ShowUsersReply (const QString&, bool isEndOf = false);
		void ShowTraceReply (const QString&, bool isEndOf = false);
		void ShowStatsReply (const QString&, bool isEndOf = false);

		void ShowBanList (const QString&,
				const QString&, const QString&, const QDateTime&);
		void ShowBanListEnd (const QString&);
		void ShowExceptList (const QString&,
				const QString&, const QString&, const QDateTime&);
		void ShowExceptListEnd (const QString&);
		void ShowInviteList (const QString&,
				const QString&, const QString&, const QDateTime&);
		void ShowInviteListEnd (const QString&);

		void SendPublicMessage (const QString& msg, const QString& channel);
		void SendPrivateMessage (IrcMessage*);
		void SendMessage2Server (const QStringList&);
		QString ParseMessageForCommand (const QString&, const QString&);
		void LeaveChannel (const QString& channel, const QString& msg);

		void ConnectToServer ();
		void DisconnectFromServer ();

		void SendCommand (const QString&);

		ServerParticipantEntry_ptr GetParticipantEntry (const QString&);
		void RemoveParticipantEntry (const QString&);

		void SetConsoleEnabled (bool);

		void ReadReply (const QByteArray&);
		void JoinFromQueue ();

		void SayCommand (const QStringList&);

		void ParseChanMode (const QString&, const QString&,
				const QString& value = QString ());
		void ParseUserMode (const QString&, const QString&);

		void ParserISupport (const QString&);
		QMap<QString, QString> GetISupport () const;

		void RequestWho (const QString&);
		void RequestWhoIs (const QString&);
		void RequestWhoWas (const QString&);

		void ClosePrivateChat (const QString& nick);

		void CreateServerParticipantEntry (QString nick);
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
