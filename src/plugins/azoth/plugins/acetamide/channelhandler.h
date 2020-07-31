/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011 Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_PLUGINS_ACETAMIDE_CHANNELHANDLER_H
#define PLUGINS_AZOTH_PLUGINS_ACETAMIDE_CHANNELHANDLER_H

#include <QObject>
#include <QHash>
#include <interfaces/azoth/imessage.h>
#include "localtypes.h"
#include "channelparticipantentry.h"

namespace LC
{
namespace Azoth
{
namespace Acetamide
{
	class ChannelCLEntry;
	class IrcMessage;
	class IrcServerHandler;
	class ChannelsManager;

	class ChannelHandler : public QObject
	{
		Q_OBJECT

		std::shared_ptr<ChannelCLEntry> ChannelCLEntry_;
		ChannelsManager *CM_;

		QString ChannelID_;
		QString Subject_;

		ChannelOptions ChannelOptions_;

		bool IsRosterReceived_;

		QHash<QString, ChannelParticipantEntry_ptr> Nick2Entry_;

		ChannelModes ChannelMode_;
		QString Url_;
	public:
		ChannelHandler (const ChannelOptions& options, ChannelsManager *manager);
		QString GetChannelID () const;
		ChannelCLEntry* GetCLEntry () const;

		ChannelsManager* GetChannelsManager () const;

		QString GetParentID () const;

		ChannelOptions GetChannelOptions () const;
		QList<QObject*> GetParticipants () const;

		ChannelParticipantEntry_ptr GetSelf ();
		ChannelParticipantEntry_ptr GetParticipantEntry (const QString&, bool announce = true);

		bool IsUserExists (const QString&) const;

		IrcMessage* CreateMessage (IMessage::Type,
				const QString&, const QString&);

		void ChangeNickname (const QString& oldNick, const QString& newNick);

		bool IsRosterReceived () const;
		void SetRosterReceived (bool);

		void HandleServiceMessage (const QString&, IMessage::Type,
				IMessage::SubType,
				ChannelParticipantEntry_ptr entry = ChannelParticipantEntry_ptr ());

		void SendPublicMessage (const QString&);
		void HandleIncomingMessage (const QString& nick, const QString& msg);
		void SetChannelUser (const QString& nick,
				const QString& user = QString (), const QString& host = QString ());

		void MakeJoinMessage (const QString&);
		void MakeLeaveMessage (const QString&, const QString&);
		void MakeKickMessage (const QString&, const QString&,
				const QString&);
		void MakePermsChangedMessage (const QString&,
				ChannelRole, bool);

		void SetMUCSubject (const QString&);
		QString GetMUCSubject () const;
		void SetTopic (const QString& topic);

		void Leave (const QString&);
		void CloseChannel ();

		void LeaveParticipant (const QString&, const QString&);

		void KickParticipant (const QString&, const QString&,
				const QString&);

		void SetRole (ChannelParticipantEntry*, const ChannelRole&, const QString&);
		void ManageWithParticipant (ChannelParticipantEntry*, const ChannelManagment&);

		void RequestBanList ();
		void RequestExceptList ();
		void RequestInviteList ();
		void AddBanListItem (QString);
		void RemoveBanListItem (QString);
		void AddExceptListItem (QString);
		void RemoveExceptListItem (QString);
		void AddInviteListItem (QString);
		void RemoveInviteListItem (QString);
		void SetBanListItem (const QString&, const QString&,
				const QDateTime&);
		void SetExceptListItem (const QString&, const QString&,
				const QDateTime&);
		void SetInviteListItem (const QString&, const QString&,
				const QDateTime&);
		ChannelModes GetChannelModes () const;
		void SetInviteMode (bool);
		void SetModerateMode (bool);
		void SetBlockOutsideMessagesMode (bool);
		void SetPrivateMode (bool);
		void SetSecretMode (bool);
		void SetServerReOpMode (bool);
		void SetOnlyOpTopicChangeMode (bool);
		void SetUserLimit (bool, int limit = 0);
		void SetChannelKey (bool, const QString& key = QString ());
		void SetNewChannelModes (const ChannelModes&);

		void UpdateEntry (const WhoMessage& message);

		void SetUrl (const QString& url);
	private:
		bool RemoveUserFromChannel (const QString&);
		ChannelParticipantEntry_ptr CreateParticipantEntry (const QString&, bool announce = true);
		void RemoveThis ();
	public slots:
		void handleWhoIs (const QString& nick);
		void handleWhoWas (const QString& nick);
		void handleWho (const QString& nick);
		void handleCTCPRequest (const QStringList& cmd);
	signals:
		void updateChanModes (const ChannelModes&);
	};
};
};
};

#endif // PLUGINS_AZOTH_PLUGINS_ACETAMIDE_CHANNELHANDLER_H
