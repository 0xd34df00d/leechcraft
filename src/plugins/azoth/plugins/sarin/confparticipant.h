/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <interfaces/azoth/iclentry.h>
#include <util/sll/void.h>
#include "types.h"

namespace LC::Azoth::Sarin
{
	class ConfEntry;
	class ToxAccount;

	class ConfParticipant
		: public QObject
		, public ICLEntry
	{
		Q_OBJECT
		Q_INTERFACES (LC::Azoth::ICLEntry)

		ToxAccount& Acc_;
		ConfEntry& Conf_;
		const Pubkey Pkey_;
		const QString EntryId_;

		QString Nick_;
	public:
		struct Online
		{
			constexpr auto operator<=> (const Online&) const = default;
		};
		struct Offline
		{
			QDateTime Since_;
			auto operator<=> (const Offline&) const = default;
		};
		using State = std::variant<Online, Offline>;
	private:
		State State_;
	public:
		explicit ConfParticipant (Pubkey pkey, QString nick, State state, ConfEntry& conf, QObject* = nullptr);

		QObject* GetQObject () override;
		IAccount* GetParentAccount () const override;
		Features GetEntryFeatures () const override;
		EntryType GetEntryType () const override;

		QString GetEntryName () const override;
		void SetEntryName (const QString& name) override;
		QString GetEntryID () const override;

		QStringList Groups () const override;
		void SetGroups (const QStringList& groups) override;
		QStringList Variants () const override;

		IMessage* CreateMessage (IMessage::Type type, const QString& variant, const QString& body) override;
		QList<IMessage*> GetAllMessages () const override;
		void PurgeMessages (const QDateTime& before) override;

		void SetChatPartState (ChatPartState state, const QString& variant) override;
		EntryStatus GetStatus (const QString& variant) const override;

		void ShowInfo () override;

		QList<QAction*> GetActions () const override;
		QMap<QString, QVariant> GetClientInfo (const QString& variant) const override;

		void MarkMsgsRead () override;
		void ChatTabClosed () override;

		void SetState (const State& state);
	signals:
		void gotMessage (QObject *msg) override;
		void statusChanged (const EntryStatus& st, const QString& variant) override;
		void availableVariantsChanged (const QStringList& newVars) override;
		void nameChanged (const QString& name) override;
		void groupsChanged (const QStringList& groups) override;
		void chatPartStateChanged (const ChatPartState& state, const QString& variant) override;
		void permsChanged () override;
		void entryGenerallyChanged () override;
	};
}
