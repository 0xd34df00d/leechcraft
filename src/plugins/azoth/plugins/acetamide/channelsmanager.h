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

#ifndef PLUGINS_AZOTH_PLUGINS_ACETAMIDE_CHANNELSMANAGER_H
#define PLUGINS_AZOTH_PLUGINS_ACETAMIDE_CHANNELSMANAGER_H

#include <memory>
#include <QObject>
#include <QHash>
#include <QSet>
#include <QQueue>
#include <QDateTime>
#include "localtypes.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	class IrcServerHandler;
	class ChannelHandler;
	class IrcAccount;

	class ChannelsManager : public QObject
	{
		Q_OBJECT

		IrcServerHandler *ISH_;

		QHash<QString, std::shared_ptr<ChannelHandler>> ChannelHandlers_;
		QSet<ChannelOptions> ChannelsQueue_;

		QString LastActiveChannel_;
	public:
		ChannelsManager (IrcServerHandler* = 0);
		IrcAccount* GetAccount () const;

		QString GetOurNick () const;

		QString GetServerID () const;

		ServerOptions GetServerOptions () const;

		QObjectList GetCLEntries () const;

		ChannelHandler* GetChannelHandler (const QString& channel);
		QList<std::shared_ptr<ChannelHandler>> GetChannels () const;

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
				const QString& user = QString (), const QString& host = QString ());

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
		bool ReceiveCmdAnswerMessage (const QString& cmd,
				const QString& answer, bool endOdfCmd = false);

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

		QMap<QString, QString> GetISupport () const;

		void SetPrivateChat (const QString& nick);

		void CreateServerParticipantEntry (QString nick);

		void UpdateEntry (const WhoMessage& message);

		int GetChannelUsersCount (const QString& channel);

		void ClosePrivateChat (const QString& nick);
	};
}
}
}

#endif // PLUGINS_AZOTH_PLUGINS_ACETAMIDE_CHANNELSMANAGER_H
