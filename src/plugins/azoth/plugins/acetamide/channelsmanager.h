/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>
#include <QHash>
#include <QSet>
#include "localtypes.h"

class QDateTime;

namespace LC::Azoth::Acetamide
{
	class IrcServerHandler;
	class ChannelHandler;
	class IrcAccount;

	using ChannelHandler_ptr = std::shared_ptr<ChannelHandler>;

	class ChannelsManager : public QObject
	{
		Q_OBJECT

		IrcServerHandler * const ISH_;

		QHash<QString, ChannelHandler_ptr> ChannelHandlers_;
		QSet<ChannelOptions> ChannelsQueue_;

		QString LastActiveChannel_;
	public:
		explicit ChannelsManager (IrcServerHandler*);

		IrcAccount* GetAccount () const;

		QString GetOurNick () const;

		QString GetServerID () const;

		ServerOptions GetServerOptions () const;

		QObjectList GetCLEntries () const;

		ChannelHandler* GetChannelHandler (const QString& channel);
		QList<ChannelHandler*> GetChannels () const;

		bool IsChannelExists (const QString& channel) const;

		int Count () const;

		QSet<ChannelOptions> GetChannelsQueue () const;
		void CleanQueue ();

		void AddChannel2Queue (const ChannelOptions& options);
		bool AddChannel (const ChannelOptions& options);

		void LeaveChannel (const QString& channel, const QString& msg);
		void CloseChannel (const QString& channel);
		void CloseAllChannels ();
		void UnregisterChannel (ChannelHandler *ich);

		QHash<QString, QObject*> GetParticipantsByNick (const QString& nick) const;

		void AddParticipant (const QString& channel, const QString& nick,
				const QString& user = {}, const QString& host = {});

		void LeaveParticipant (const QString& channel,
				const QString& nick, const QString& msg);
		void QuitParticipant (const QString& nick, const QString& msg);

		void KickParticipant (const QString& channel, const QString& target,
				const QString& reason, const QString& who);
		void KickCommand (const QString& channel,
				const QString& nick, const QString& reason);

		void ChangeNickname (const QString& oldNick, const QString& newNick);

		void GotNames (const QString& channel, const QStringList& participants);
		void GotEndOfNamesCmd (const QString& channel);

		void SendPublicMessage (const QString& channel, const QString& msg);

		void ReceivePublicMessage (const QString& channel,
				const QString& nick, const QString& msg);
		bool ReceiveCmdAnswerMessage (const QString& answer);

		void SetMUCSubject (const QString& channel, const QString& topic);
		void SetTopic (const QString& channel, const QString& topic);

		void CTCPReply (const QString& msg);
		void CTCPRequestResult (const QString& msg);

		void SetBanListItem (const QString& channel,
				const QString& mask, const QString& nick, const QDateTime& time);
		void RequestBanList (const QString& channel);
		void AddBanListItem (const QString& channel, const QString& mask);
		void RemoveBanListItem (const QString& channel, const QString& mask);

		void SetExceptListItem (const QString& channel,
				const QString& mask, const QString& nick, const QDateTime& time);
		void RequestExceptList (const QString& channel);
		void AddExceptListItem (const QString& channel, const QString& mask);
		void RemoveExceptListItem (const QString& channel, const QString& mask);

		void SetInviteListItem (const QString& channel,
				const QString& mask, const QString& nick, const QDateTime& time);
		void RequestInviteList (const QString& channel);
		void AddInviteListItem (const QString& channel, const QString& mask);
		void RemoveInviteListItem (const QString& channel, const QString& mask);

		void ParseChanMode (const QString& channel,
				const QString& mode, const QString& value);
		void SetNewChannelMode (const QString& channel,
				const QString& mode, const QString& name);
		void SetNewChannelModes (const QString& channel, const ChannelModes& modes);

		void RequestWhoIs (const QString& channel, const QString& nick);
		void RequestWhoWas (const QString& channel, const QString& nick);
		void RequestWho (const QString& channel, const QString& nick);

		void CTCPRequest (const QStringList& cmd, const QString& channel);

		QHash<QString, QString> GetISupport () const;

		void SetPrivateChat (const QString& nick) const;

		void CreateServerParticipantEntry (const QString& nick);

		void UpdateEntry (const WhoMessage& message);

		int GetChannelUsersCount (const QString& channel);

		void ClosePrivateChat (const QString& nick);

		void SetChannelUrl (const QString& channel, const QString& url);
		void SetTopicWhoTime (const QString& channel,
				const QString& who, quint64 time);
	};
}
