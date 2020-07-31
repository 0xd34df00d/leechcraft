/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <unordered_map>
#include <QObject>
#include <QSet>
#include <util/sll/util.h>
#include <interfaces/azoth/iclentry.h>
#include <interfaces/azoth/imucentry.h>
#include "structures.h"
#include "entrybase.h"

namespace LC
{
namespace Azoth
{
namespace Murm
{
	class VkAccount;
	class VkEntry;

	class VkChatEntry : public EntryBase
					  , public IMUCEntry
	{
		Q_OBJECT
		Q_INTERFACES (LC::Azoth::IMUCEntry)

		ChatInfo Info_;
		QSet<int> PendingUserInfoRequests_;

		std::unordered_map<VkEntry*, Util::DefaultScopeGuard> EntriesGuards_;
	public:
		enum class HandleMessageResult
		{
			Accepted,
			Rejected,
			UserInfoRequested
		};

		VkChatEntry (const ChatInfo&, VkAccount*);

		void Send (VkMessage*) override;
		HandleMessageResult HandleMessage (const MessageInfo&, const FullMessageInfo&);

		const ChatInfo& GetInfo () const;
		void UpdateInfo (const ChatInfo&);

		void HandleAdded (const UserInfo&);
		void HandleRemoved (qulonglong);

		Features GetEntryFeatures () const override;
		EntryType GetEntryType () const override;
		QString GetEntryName () const override;
		void SetEntryName (const QString& name) override;
		QString GetEntryID () const override;
		QString GetHumanReadableID () const override;
		QStringList Groups () const override;
		void SetGroups (const QStringList& groups) override;
		QStringList Variants () const override;
		void SetChatPartState (ChatPartState state, const QString& variant) override;
		EntryStatus GetStatus (const QString& variant = QString ()) const override;
		void ShowInfo () override;
		QList<QAction*> GetActions () const override;
		QMap<QString, QVariant> GetClientInfo (const QString&) const override;
		void ChatTabClosed () override;

		MUCFeatures GetMUCFeatures () const override;
		QString GetMUCSubject () const override;
		bool CanChangeSubject () const override;
		void SetMUCSubject (const QString& subject) override;
		QList<QObject*> GetParticipants () override;
		bool IsAutojoined () const override;
		void Join () override;
		void Leave (const QString&) override;
		QString GetNick () const override;
		void SetNick (const QString&) override;
		QString GetGroupName () const override;
		QString GetRealID (QObject*) const override;
		QVariantMap GetIdentifyingData () const override;
		void InviteToMUC (const QString& userId, const QString& msg) override;
	private slots:
		void handleGotUsers (const QList<UserInfo>&);
	signals:
		void gotNewParticipants (const QList<QObject*>& parts) override;
		void mucSubjectChanged (const QString& newSubj) override;
		void nicknameConflict (const QString& usedNick) override;
		void beenKicked (const QString& reason) override;
		void beenBanned (const QString& reason) override;

		void removeEntry ();
	};
}
}
}
