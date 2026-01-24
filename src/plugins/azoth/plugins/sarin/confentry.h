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
	class ConfMessage;
	class ConfParticipant;
	class ConfsManager;

	class ConfEntry final
		: public QObject
		, public ICLEntry
		, public IMUCEntry
	{
		Q_OBJECT
		Q_INTERFACES (LC::Azoth::ICLEntry LC::Azoth::IMUCEntry)

		ConfsManager& Mgr_;

		const ConfId ConfId_;
		const uint32_t ConfNum_;

		const QString EntryId_;

		QHash<Pubkey, ConfParticipant*> Participants_;
		QHash<uint32_t, Pubkey> OnlineNum2Pubkey_;
		QHash<uint32_t, Pubkey> OfflineNum2Pubkey_;

		QList<IMessage*> AllMessages_;
	public:
		explicit ConfEntry (uint32_t confNum, ConfId confId, ConfsManager& mgr);
		~ConfEntry () override;

		ConfsManager& GetConfsManager ();

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

		void SendMessage (const OutgoingMessage&) override;
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

		void HandleConnected ();
		void HandleMessage (const ConfMessageEvent&);
		Util::ContextTask<void> RefreshParticipants ();

		void AppendMessage (ConfMessage*);
	private:
		Util::ContextTask<void> RunLeave ();
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
