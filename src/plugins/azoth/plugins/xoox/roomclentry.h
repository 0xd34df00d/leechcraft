/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>
#include <QStringList>
#include <QXmppMucIq.h>
#include <interfaces/azoth/iclentry.h>
#include <interfaces/azoth/imucentry.h>
#include <interfaces/azoth/imucperms.h>
#include <interfaces/azoth/iconfigurablemuc.h>
#include "roomparticipantentry.h"
#include "glooxaccount.h"

class QXmppBookmarkSet;

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	class RoomPublicMessage;
	class RoomHandler;

	class RoomCLEntry : public QObject
					  , public ICLEntry
					  , public IMUCEntry
					  , public IMUCPerms
					  , public IConfigurableMUC
					  , public IHaveDirectedStatus
	{
		Q_OBJECT
		Q_INTERFACES (LC::Azoth::ICLEntry
						LC::Azoth::IMUCEntry
						LC::Azoth::IMUCPerms
						LC::Azoth::IConfigurableMUC
						LC::Azoth::IHaveDirectedStatus)

		friend class RoomHandler;

		const bool IsAutojoined_;
		GlooxAccount *Account_;
		QList<IMessage*> AllMessages_;
		RoomHandler *RH_;

		const QMap<QByteArray, QList<QByteArray>> Perms_;
		const QMap<QXmppMucItem::Role, QByteArray> Role2Str_;
		const QMap<QXmppMucItem::Affiliation, QByteArray> Aff2Str_;
		const QMap<QByteArray, QString> Translations_;

		mutable QAction *ActionRequestVoice_ = nullptr;
	public:
		RoomCLEntry (RoomHandler*, bool, GlooxAccount*);

		RoomHandler* GetRoomHandler () const;

		// ICLEntry
		QObject* GetQObject ();
		GlooxAccount* GetParentAccount () const ;
		Features GetEntryFeatures () const;
		EntryType GetEntryType () const;
		QString GetEntryName () const;
		void SetEntryName (const QString&);
		QString GetEntryID () const;
		QString GetHumanReadableID () const;
		QStringList Groups () const;
		void SetGroups (const QStringList&);
		QStringList Variants () const;
		IMessage* CreateMessage (IMessage::Type,
				const QString&, const QString&);
		QList<IMessage*> GetAllMessages () const;
		void PurgeMessages (const QDateTime&);
		void SetChatPartState (ChatPartState, const QString&);
		EntryStatus GetStatus (const QString&) const;
		QList<QAction*> GetActions () const;
		void ShowInfo ();
		QMap<QString, QVariant> GetClientInfo (const QString&) const;
		void MarkMsgsRead ();
		void ChatTabClosed ();

		// IMUCEntry
		MUCFeatures GetMUCFeatures () const;
		QString GetMUCSubject () const;
		bool CanChangeSubject () const;
		void SetMUCSubject (const QString&);
		QList<QObject*> GetParticipants ();
		bool IsAutojoined () const;
		void Join ();
		void Leave (const QString&);
		QString GetNick () const;
		void SetNick (const QString&);
		QString GetGroupName () const;
		QVariantMap GetIdentifyingData () const;
		QString GetRealID (QObject*) const;
		void InviteToMUC (const QString&, const QString&);

		// IMUCPerms
		QMap<QByteArray, QList<QByteArray>> GetPossiblePerms () const;
		QMap<QByteArray, QList<QByteArray>> GetPerms (QObject *object) const;
		QPair<QByteArray, QByteArray> GetKickPerm () const;
		QPair<QByteArray, QByteArray> GetBanPerm () const;
		QByteArray GetAffName (QObject*) const;
		bool MayChangePerm (QObject*, const QByteArray&, const QByteArray&) const;
		void SetPerm (QObject*, const QByteArray&, const QByteArray&, const QString&);
		void TrySetPerm (const QString&, const QByteArray&, const QByteArray&, const QString&);
		bool IsLessByPerm (QObject*, QObject*) const;
		bool IsMultiPerm (const QByteArray&) const;
		QString GetUserString (const QByteArray&) const;

		// IConfigurableMUC
		QWidget* GetConfigurationWidget ();
		void AcceptConfiguration (QWidget*);

		// IHaveDirectedStatus
		bool CanSendDirectedStatusNow (const QString&);
		void SendDirectedStatus (const EntryStatus&, const QString&);

		void MoveMessages (const RoomParticipantEntry_ptr& from, const RoomParticipantEntry_ptr& to);

		void HandleMessage (RoomPublicMessage*);
		void HandleNewParticipants (const QList<ICLEntry*>&);
		void HandleSubjectChanged (const QString&);
	private slots:
		void handleBookmarks (const QXmppBookmarkSet&);
		void reemitStatusChange (const EntryStatus&);
	signals:
		void gotMessage (QObject*);
		void statusChanged (const EntryStatus&, const QString&);
		void availableVariantsChanged (const QStringList&);
		void nameChanged (const QString&);
		void groupsChanged (const QStringList&);
		void chatPartStateChanged (const ChatPartState&, const QString&);
		void permsChanged ();
		void entryGenerallyChanged ();

		void gotNewParticipants (const QList<QObject*>&);
		void mucSubjectChanged (const QString&);
		void nicknameConflict (const QString&);
		void beenKicked (const QString&);
		void beenBanned (const QString&);
	};
}
}
}
