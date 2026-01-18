/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <util/threads/coro/taskfwd.h>
#include <interfaces/azoth/iclentry.h>
#include <interfaces/azoth/imucentry.h>
#include "types.h"

namespace LC::Azoth::Sarin
{
	class GroupsManager;

	class GroupChatEntry
		: public QObject
		, public ICLEntry
		, public IMUCEntry
	{
		Q_OBJECT
		Q_INTERFACES (LC::Azoth::ICLEntry LC::Azoth::IMUCEntry)

		GroupsManager& Mgr_;

		const QString GroupId_;
		QString Nick_;

		const uint32_t GroupNum_;
	public:
		explicit GroupChatEntry (const QString& nick, uint32_t groupNum, const QString& groupId, GroupsManager& mgr);

		QObject* GetQObject () override;
		IAccount* GetParentAccount () const override;
		Features GetEntryFeatures () const override;
		EntryType GetEntryType () const override;

		QString GetEntryName () const override;
		void SetEntryName (const QString&) override;

		QString GetEntryID () const override;

		QStringList Groups () const override;
		void SetGroups (const QStringList&) override;

		QStringList Variants () const override;

		IMessage* CreateMessage (IMessage::Type type, const QString& variant, const QString& body) override;
		QList<IMessage*> GetAllMessages () const override;
		void PurgeMessages (const QDateTime& before) override;

		void SetChatPartState (ChatPartState, const QString&) override;

		EntryStatus GetStatus (const QString&) const override;
		void ShowInfo () override;
		QList<QAction*> GetActions () const override;
		QMap<QString, QVariant> GetClientInfo (const QString&) const override;
		void MarkMsgsRead () override;
		void ChatTabClosed () override;

		MUCFeatures GetMUCFeatures () const override;
		QString GetMUCSubject () const override;
		void SetMUCSubject (const QString& subject) override;
		bool CanChangeSubject () const override;
		QList<QObject*> GetParticipants () override;
		bool IsAutojoined () const override;
		void Join () override;
		void Leave (const QString& msg) override;
		QString GetNick () const override;
		void SetNick (const QString& nick) override;
		QString GetGroupName () const override;
		QString GetRealID (QObject *participant) const override;
		QVariantMap GetIdentifyingData () const override;
		void InviteToMUC (const QString& userId, const QString& msg) override;

		void HandlePeerJoined (uint32_t peerId);
		void HandlePeerExited (uint32_t peerId, const GroupPeerExitedEvent&);
	private:
		Util::ContextTask<void> RunLeave (QString, int retry = 0);
		Util::ContextTask<void> RunSetNick (QString, int retry = 0);
	signals:
		void gotMessage (QObject*) override;
		void statusChanged (const EntryStatus&, const QString&) override;
		void availableVariantsChanged (const QStringList&) override;
		void nameChanged (const QString&) override;
		void groupsChanged (const QStringList&) override;
		void chatPartStateChanged (const ChatPartState&, const QString&) override;
		void permsChanged () override;
		void entryGenerallyChanged () override;

		void gotNewParticipants (const QList<QObject*>&) override;
		void mucSubjectChanged (const QString&) override;
		void nicknameConflict (const QString&) override;
		void beenKicked (const QString&) override;
		void beenBanned (const QString&) override;
	};
}
