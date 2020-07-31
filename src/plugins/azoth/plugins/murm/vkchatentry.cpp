/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "vkchatentry.h"
#include <QStringList>
#include <QtDebug>
#include <interfaces/azoth/iproxyobject.h>
#include "vkaccount.h"
#include "vkentry.h"
#include "vkmessage.h"
#include "vkconnection.h"

namespace LC
{
namespace Azoth
{
namespace Murm
{
	VkChatEntry::VkChatEntry (const ChatInfo& chatInfo, VkAccount *acc)
	: EntryBase (acc)
	, Info_ (chatInfo)
	{
		for (const auto& info : chatInfo.Users_)
			if (const auto entry = acc->GetEntryOrCreate (info))
				EntriesGuards_.emplace (entry, entry->RegisterIn (this));

		connect (acc->GetConnection (),
				SIGNAL (gotUsers (QList<UserInfo>)),
				this,
				SLOT (handleGotUsers (QList<UserInfo>)));
	}

	void VkChatEntry::Send (VkMessage *msg)
	{
		Account_->Send (GetInfo ().ChatID_, VkConnection::Type::Chat, msg);
	}

	VkChatEntry::HandleMessageResult VkChatEntry::HandleMessage (const MessageInfo& info, const FullMessageInfo& full)
	{
		const auto from = info.Params_ ["from"].toULongLong ();
		if (from == 0)
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown from"
					<< info.Params_;
			return HandleMessageResult::Rejected;
		}

		const auto entry = Account_->GetEntry (from);
		if (!entry)
		{
			qDebug () << Q_FUNC_INFO
					<< "unknown entry for"
					<< from;
			if (!PendingUserInfoRequests_.contains (from))
			{
				qDebug () << Q_FUNC_INFO
						<< "requesting user info for"
						<< from;
				Account_->GetConnection ()->GetUserInfo ({ from });
				PendingUserInfoRequests_ << from;
			}
			return HandleMessageResult::UserInfoRequested;
		}

		HasUnread_ = true;

		auto msg = new VkMessage (false, IMessage::Direction::In, IMessage::Type::MUCMessage, this, entry);
		msg->SetDateTime (info.TS_);
		msg->SetID (info.ID_);

		HandleAttaches (msg, info, full);

		Store (msg);

		return HandleMessageResult::Accepted;
	}

	const ChatInfo& VkChatEntry::GetInfo () const
	{
		return Info_;
	}

	void VkChatEntry::UpdateInfo (const ChatInfo& info)
	{
		for (const auto& userInfo : info.Users_)
			if (std::none_of (Info_.Users_.begin (), Info_.Users_.end (),
					[userInfo] (const UserInfo& other) { return other.ID_ == userInfo.ID_; }))
				HandleAdded (userInfo);
		for (const auto& userInfo : Info_.Users_)
			if (std::none_of (info.Users_.begin (), info.Users_.end (),
					[userInfo] (const UserInfo& other) { return other.ID_ == userInfo.ID_; }))
				HandleRemoved (userInfo.ID_);

		const bool titleChanged = info.Title_ != Info_.Title_;

		Info_ = info;
		if (titleChanged)
		{
			emit nameChanged (GetEntryName ());

			for (const auto& info : Info_.Users_)
				if (const auto entry = Account_->GetEntry (info.ID_))
					entry->ReemitGroups ();
		}
	}

	void VkChatEntry::HandleAdded (const UserInfo& info)
	{
		if (const auto entry = Account_->GetEntryOrCreate (info))
		{
			if (EntriesGuards_.find (entry) != EntriesGuards_.end ())
				qWarning () << Q_FUNC_INFO
						<< "entry for"
						<< info.ID_
						<< "is already added";
			else
				EntriesGuards_.emplace (entry, entry->RegisterIn (this));
		}
	}

	void VkChatEntry::HandleRemoved (qulonglong id)
	{
		if (id == Account_->GetSelf ()->GetInfo ().ID_)
			emit removeEntry ();
		else if (const auto entry = Account_->GetEntry (id))
		{
			const auto pos = EntriesGuards_.find (entry);
			if (pos != EntriesGuards_.end ())
				EntriesGuards_.erase (pos);
		}
	}

	ICLEntry::Features VkChatEntry::GetEntryFeatures () const
	{
		return FSessionEntry | FSupportsRenames;
	}

	ICLEntry::EntryType VkChatEntry::GetEntryType () const
	{
		return EntryType::MUC;
	}

	QString VkChatEntry::GetEntryName () const
	{
		return Info_.Title_;
	}

	void VkChatEntry::SetEntryName (const QString& title)
	{
		Account_->GetConnection ()->SetChatTitle (Info_.ChatID_, title);
	}

	QString VkChatEntry::GetEntryID () const
	{
		return Account_->GetAccountID () + QString::number (Info_.ChatID_);
	}

	QString VkChatEntry::GetHumanReadableID () const
	{
		return QString::number (Info_.ChatID_);
	}

	QStringList VkChatEntry::Groups () const
	{
		return { tr ("Chats") };
	}

	void VkChatEntry::SetGroups (const QStringList&)
	{
	}

	QStringList VkChatEntry::Variants () const
	{
		return {};
	}

	void VkChatEntry::SetChatPartState (ChatPartState, const QString&)
	{
		// TODO
	}

	EntryStatus VkChatEntry::GetStatus (const QString&) const
	{
		return { SOnline, {} };
	}

	void VkChatEntry::ShowInfo ()
	{
	}

	QList<QAction*> VkChatEntry::GetActions () const
	{
		return {};
	}

	QMap<QString, QVariant> VkChatEntry::GetClientInfo (const QString&) const
	{
		return {};
	}

	void VkChatEntry::ChatTabClosed ()
	{
	}

	IMUCEntry::MUCFeatures VkChatEntry::GetMUCFeatures () const
	{
		return MUCFCanInvite;
	}

	QString VkChatEntry::GetMUCSubject () const
	{
		return {};
	}

	bool VkChatEntry::CanChangeSubject () const
	{
		return false;
	}

	void VkChatEntry::SetMUCSubject (const QString&)
	{
	}

	QList<QObject*> VkChatEntry::GetParticipants ()
	{
		QList<QObject*> result;
		for (const auto& info : Info_.Users_)
			if (const auto entry = Account_->GetEntry (info.ID_))
				result << entry;
		return result;
	}

	bool VkChatEntry::IsAutojoined () const
	{
		return false;
	}

	void VkChatEntry::Join ()
	{
	}

	void VkChatEntry::Leave (const QString&)
	{
		const auto selfId = Account_->GetSelf ()->GetInfo ().ID_;
		Account_->GetConnection ()->RemoveChatUser (Info_.ChatID_, selfId);
	}

	QString VkChatEntry::GetNick () const
	{
		return Account_->GetOurNick ();
	}

	void VkChatEntry::SetNick (const QString&)
	{
	}

	QString VkChatEntry::GetGroupName () const
	{
		return tr ("Participants of %1").arg (Info_.Title_);
	}

	QString VkChatEntry::GetRealID (QObject*) const
	{
		return {};
	}

	QVariantMap VkChatEntry::GetIdentifyingData () const
	{
		return {};
	}

	void VkChatEntry::InviteToMUC (const QString& userId, const QString&)
	{
		bool ok = false;
		const auto numericId = userId.toULongLong (&ok);

		if (!ok)
		{
			qWarning () << Q_FUNC_INFO
					<< "incorrect user id"
					<< userId;
			return;
		}

		Account_->GetConnection ()->AddChatUser (Info_.ChatID_, numericId);
	}

	void VkChatEntry::handleGotUsers (const QList<UserInfo>& infos)
	{
		for (const auto& info : infos)
			PendingUserInfoRequests_.remove (info.ID_);
	}
}
}
}
