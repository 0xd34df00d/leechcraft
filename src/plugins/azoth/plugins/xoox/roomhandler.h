/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_ROOMHANDLER_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_ROOMHANDLER_H
#include <QObject>
#include <QHash>
#include <interfaces/azoth/imucentry.h>
#include "clientconnection.h"
#include "roomparticipantentry.h"

class QXmppVCardIq;
class QXmppMucManager;
class QXmppMucRoom;

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	class RoomCLEntry;
	class GlooxAccount;
	class RoomParticipantEntry;
	class FormBuilder;

	class RoomHandler : public QObject
	{
		Q_OBJECT

		GlooxAccount *Account_;
		QXmppMucManager *MUCManager_;
		const QString RoomJID_;
		QXmppMucRoom *Room_;
		RoomCLEntry *CLEntry_;
		QHash<QString, RoomParticipantEntry_ptr> Nick2Entry_;
		QString Subject_;
		// contains new nicks
		QSet<QString> PendingNickChanges_;
		bool HadRequestedPassword_;

		QXmppDiscoveryIq ServerDisco_;
	public:
		RoomHandler (const QString& roomJID, const QString& ourNick,
				bool asAutojoin, GlooxAccount*);

		QString GetRoomJID () const;
		RoomCLEntry* GetCLEntry ();

		void SetPresence (QXmppPresence);

		GlooxMessage* CreateMessage (IMessage::Type,
				const QString&, const QString&);
		QList<QObject*> GetParticipants () const;
		QString GetSubject () const;
		void SetSubject (const QString&);
		void Join ();
		void Leave (const QString& msg, bool remove = true);
		RoomParticipantEntry* GetSelf ();
		QString GetOurNick () const;
		void SetOurNick (const QString&);

		void SetAffiliation (RoomParticipantEntry*,
				QXmppMucItem::Affiliation, const QString&);
		void SetRole (RoomParticipantEntry*,
				QXmppMucItem::Role, const QString&);

		QXmppMucRoom* GetRoom () const;

		void HandleErrorPresence (const QXmppPresence&, const QString&);
		void HandlePermsChanged (const QString&,
				QXmppMucItem::Affiliation,
				QXmppMucItem::Role,
				const QString&);
		void HandleMessage (const QXmppMessage&, const QString&);
		/** Creates a new entry for the given nick if it
		 * doesn't exist already (and does so by calling
		 * CreateParticipantEntry()) or just returns the
		 * already existing one.
		 */
		RoomParticipantEntry_ptr GetParticipantEntry (const QString& nick, bool announce = true);

		bool IsGateway () const;
	private slots:
		void handleParticipantAdded (const QString&);
		void handleParticipantChanged (const QString&);
		void handleParticipantRemoved (const QString&);

		void requestVoice ();

		void handleChatTabClosed ();
	private:
		void HandleMessageExtensions (const QXmppMessage&);
		void HandlePendingForm (std::unique_ptr<QXmppDataForm>, const QString&);

		void HandleRenameStart (const RoomParticipantEntry_ptr& entry,
				const QString& nick, const QString& newNick);

		/** Creates a new entry for the given nick.
		 */
		RoomParticipantEntry_ptr CreateParticipantEntry (const QString& nick, bool announce);
		void MakeLeaveMessage (const QXmppPresence&, const QString&);
		void MakeJoinMessage (const QXmppPresence&, const QString&);
		void MakeStatusChangedMessage (const QXmppPresence&, const QString&);
		void MakeNickChangeMessage (const QString&, const QString&);
		void MakeKickMessage (const QString&, const QString&);
		void MakeBanMessage (const QString&, const QString&);
		void MakePermsChangedMessage (const QString&,
				QXmppMucItem::Affiliation,
				QXmppMucItem::Role,
				const QString&);
		void HandleNickConflict ();
		void HandlePasswordRequired ();
		QString GetPassKey () const;

		void RemoveEntry (RoomParticipantEntry*);

		void RemoveThis ();
	signals:
		void gotPendingForm (QXmppDataForm*, const QString&);
	};
}
}
}

#endif
