/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCSERVERHANDLER_H
#define PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCSERVERHANDLER_H

#include <QObject>
#include <QTcpSocket>
#include <interfaces/azoth/imessage.h>
#include "localtypes.h"
#include "invitechannelsdialog.h"
#include "serverparticipantentry.h"
#include "serverresponsemanager.h"
#include "usercommandmanager.h"

namespace LC
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
	class ServerResponseManager;
	class RplISupportParser;
	class ChannelsManager;

	const int AnswersOnWhoCommand = 2;

	class IrcServerHandler : public QObject
	{
		Q_OBJECT

		IrcAccount * const Account_;
		IrcErrorHandler * const ErrorHandler_;
		IrcParser * const IrcParser_;
		IrcServerCLEntry * const ServerCLEntry_;
		std::unique_ptr<IrcServerSocket> Socket_;
		UserCommandManager CmdManager_;
		ServerResponseManager ServerResponseManager_;
		ChannelsManager * const ChannelsManager_;

		ConnectionState ServerConnectionState_ = NotConnected;
		bool IsConsoleEnabled_ = false;
		QString ServerID_;
		QString NickName_;
		ServerOptions ServerOptions_;
		std::unique_ptr<InviteChannelsDialog> InviteChannelsDialog_;
		QHash<QString, ServerParticipantEntry_ptr> Nick2Entry_;
		QHash<QByteArray, QVariant> ISupport_;

		QHash<QString, int> SpyWho_;
		QHash<QString, WhoIsMessage> SpyNick2WhoIsMessage_;
		QTimer * const AutoWhoTimer_;
		
		int LastNickIndex_ = 0;
	public:
		IrcServerHandler (const ServerOptions& server, IrcAccount* account);
		~IrcServerHandler () override;
		
		IrcServerCLEntry* GetCLEntry () const;
		IrcAccount* GetAccount () const;
		IrcParser* GetParser () const;
		ChannelsManager* GetChannelManager () const;
		QString GetNickName () const;
		QString GetServerID () const;
		ServerOptions GetServerOptions () const;
		QObjectList GetCLEntries () const;

		QList<ChannelHandler*> GetChannelHandlers () const;

		IrcMessage* CreateMessage (IMessage::Type type,
				const QString& variant, const QString& body);

		bool IsChannelExists (const QString& channel) const;

		void SetNickName (const QString& nick);
		void Add2ChannelsQueue (const ChannelOptions& options);

		void JoinChannel (const ChannelOptions& options);
		bool JoinedChannel (const ChannelOptions& options);
		void JoinParticipant (const QString& nick, const QString& msg,
				const QString& user = QString (), const QString& host = QString ());

		void CloseChannel (const QString& channel);
		void LeaveParticipant (const QString& nick,
				const QString& channel, const QString& msg);

		void SendQuit ();
		void QuitParticipant (const QString& nick, const QString& msg);

		void SendMessage (const QStringList&);
		void IncomingMessage (const QString& nick,
				const QString& target, const QString& msg,
				IMessage::Type type = IMessage::Type::ChatMessage);
		void IncomingNoticeMessage (const QString&, const QString&);

		void ChangeNickname (const QString&, const QString&);

		void GetBanList (const QString&);
		void GetExceptList (const QString&);
		void GetInviteList (const QString&);
		void AddBanListItem (const QString&, const QString&);
		void RemoveBanListItem (const QString&, const QString&);
		void AddExceptListItem (const QString&, const QString&);
		void RemoveExceptListItem (const QString&, const QString&);
		void AddInviteListItem (const QString&, const QString&);
		void RemoveInviteListItem (const QString&, const QString&);
		void SetNewChannelModes (const QString&, const ChannelModes&);
		void SetNewChannelMode (const QString&, const QString&, const QString&);

		void PongMessage (const QString&);

		void SetTopic (const QString& channel, const QString& topic);
		void GotTopic (const QString&, const QString&);
		void GotKickCommand (const QString&, const QString&,
				const QString&, const QString&);
		void KickParticipant (const QString&, const QString&,
				const QString&);
		void GotInvitation (const QString&, const QString&);
		void ShowAnswer (const QByteArray& cmd,
				const QString& answer, bool isEndOf = false,
				IMessage::Type type = IMessage::Type::EventMessage);

		void CTCPReply (const QString&, const QString&, const QString&);
		void CTCPRequestResult (const QString&);
		void CTCPRequst (const QStringList& cmd);

		void GotNames (const QString&, const QStringList&);
		void GotEndOfNames (const QString&);

		void ShowUserHost (const QString&, const QString&);
		void ShowIsUserOnServer (const QString&);

		void ShowWhoIsReply (const WhoIsMessage& msg, bool isEndOf = false);
		void ShowWhoWasReply (const QString&, bool isEndOf = false);
		void ShowWhoReply (const WhoMessage& msg, bool isEndOf = false);
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
		void SendMessage2Server (const QString&);
		QString ParseMessageForCommand (const QString&, const QString&) const;
		void LeaveChannel (const QString& channel, const QString& msg);

		void ConnectToServer ();
		void DisconnectFromServer ();

		void SendCommand (const QString&) const;

		ServerParticipantEntry_ptr GetParticipantEntry (const QString&);
		void RemoveParticipantEntry (const QString&);

		void SetConsoleEnabled (bool);

		void ReadReply (const QString&);
		void JoinFromQueue ();

		void SayCommand (const QStringList&);

		void ParseChanMode (const QString&, const QString&,
				const QString& value = QString ());
		void ParseUserMode (const QString&, const QString&);

		void ParserISupport (const QString&);
		QHash<QByteArray, QVariant> GetISupport () const;

		void RequestWho (const QString&);
		void RequestWhoIs (const QString&);
		void RequestWhoWas (const QString&);

		void ClosePrivateChat (const QString& nick);

		void CreateServerParticipantEntry (const QString& nick);

		void VCardRequest (const QString& nick);

		void SetAway (const QString& message);
		void ChangeAway (bool away, const QString& message = QString ());

		void GotChannelUrl (const QString& channel, const QString& url);
		void GotTopicWhoTime (const QString& channel,
				const QString& who, quint64 time);

		void SetIrcServerInfo (IrcServer server, const QString& version);

		void GotChannelsListBegin (const IrcMessageOptions& opts);
		void GotChannelsList (const IrcMessageOptions& opts);
		void GotChannelsListEnd (const IrcMessageOptions& opts);

	private:
		void HandleSpyNick (const WhoIsMessage&);
		void SendToConsole (IMessage::Direction, const QString&) const;
		void NickCmdError ();
		ServerParticipantEntry_ptr CreateParticipantEntry (const QString&);
	public slots:
		void autoWhoRequest ();
		void showChannels (const QStringList& = QStringList ());
	private slots:
		void joinAfterInvite ();
		void handleSetAutoWho ();
		void handleUpdateWhoPeriod ();
	signals:
		void connected (const QString&);
		void disconnected (const QString&);
		void sendMessageToConsole (IMessage::Direction, const QString&) const;

		void gotChannelsBegin ();
		void gotChannels (const ChannelsDiscoverInfo& info);
		void gotChannelsEnd ();
	};
};
};
};
#endif // PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCSERVERHANDLER_H
