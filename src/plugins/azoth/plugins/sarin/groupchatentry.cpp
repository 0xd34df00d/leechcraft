/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "groupchatentry.h"
#include "groupsmanager.h"
#include "toxaccount.h"
#include "toxthread.h"

namespace LC::Azoth::Sarin
{
	GroupChatEntry::GroupChatEntry (const QString& nick, uint32_t groupNum, const QString& groupId, GroupsManager& mgr)
	: QObject { &mgr }
	, Mgr_ { mgr }
	, GroupId_ { groupId }
	, Nick_ { nick }
	, GroupNum_ {  groupNum }
	{
	}

	QObject* GroupChatEntry::GetQObject ()
	{
		return this;
	}

	IAccount* GroupChatEntry::GetParentAccount () const
	{
		return &Mgr_.GetAccount ();
	}

	ICLEntry::Features GroupChatEntry::GetEntryFeatures () const
	{
		return {};
	}

	ICLEntry::EntryType GroupChatEntry::GetEntryType () const
	{
		return EntryType::MUC;
	}

	QString GroupChatEntry::GetEntryName () const
	{
		return GroupId_;
	}

	void GroupChatEntry::SetEntryName (const QString&)
	{
	}

	QString GroupChatEntry::GetEntryID () const
	{
		return GroupId_;
	}

	QStringList GroupChatEntry::Groups () const
	{
		return {};
	}

	void GroupChatEntry::SetGroups (const QStringList&)
	{
	}

	QStringList GroupChatEntry::Variants () const
	{
		return { {} };
	}

	void GroupChatEntry::SendMessage (const OutgoingMessage& message)
	{
		// TODO
	}

	QList<IMessage*> GroupChatEntry::GetAllMessages () const
	{
		// TODO
		return {};
	}

	void GroupChatEntry::PurgeMessages (const QDateTime& before)
	{
		//AzothUtil::StandardPurgeMessages (AllMessages_, before);
	}

	void GroupChatEntry::SetChatPartState (ChatPartState, const QString&)
	{
	}

	EntryStatus GroupChatEntry::GetStatus (const QString&) const
	{
		return { SOnline, {} };
	}

	void GroupChatEntry::ShowInfo ()
	{
	}

	QList<QAction*> GroupChatEntry::GetActions () const
	{
		return {};
	}

	QMap<QString, QVariant> GroupChatEntry::GetClientInfo (const QString&) const
	{
		return {};
	}

	void GroupChatEntry::MarkMsgsRead ()
	{
	}

	void GroupChatEntry::ChatTabClosed ()
	{
	}

	IMUCEntry::MUCFeatures GroupChatEntry::GetMUCFeatures () const
	{
		return MUCFCanHaveSubject;
	}

	QString GroupChatEntry::GetMUCSubject () const
	{
		// TODO
		return {};
	}

	void GroupChatEntry::SetMUCSubject (const QString& subject)
	{
		// TODO
	}

	bool GroupChatEntry::CanChangeSubject () const
	{
		// TODO
		return false;
	}

	QList<QObject*> GroupChatEntry::GetParticipants ()
	{
		return {};
	}

	bool GroupChatEntry::IsAutojoined () const
	{
		return false;
	}

	void GroupChatEntry::Join ()
	{
	}

	QString GroupChatEntry::GetGroupName () const
	{
		return {};
	}

	QString GroupChatEntry::GetRealID (QObject *participant) const
	{
		return {};
	}

	QVariantMap GroupChatEntry::GetIdentifyingData () const
	{
		// TODO
		return {};
	}

	void GroupChatEntry::InviteToMUC (const QString& userId, const QString& msg)
	{
	}

	void GroupChatEntry::HandlePeerJoined (uint32_t peerId)
	{
		qDebug () << peerId << "joined";
	}

	void GroupChatEntry::HandlePeerExited (uint32_t peerId, const GroupPeerExitedEvent&)
	{
		qDebug () << peerId << "exited";
	}

	Util::ContextTask<void> GroupChatEntry::RunLeave (QString msg, int retry)
	{
		co_await Util::AddContextObject { *this };

		const auto tox = Mgr_.GetAccount ().GetTox ();
		if (!tox)
			co_return;

		const auto result = co_await tox->RunWithStrError (&tox_group_leave, msg, GroupNum_);
		if (result.IsRight ())
		{
			Mgr_.HandleLeft (GroupNum_);
			co_return;
		}

		qWarning () << "failed to leave:" << tox_err_group_leave_to_string (result.GetLeft ());
		switch (result.GetLeft ())
		{
		case TOX_ERR_GROUP_LEAVE_OK:
			// shouldn't happen
			break;
		case TOX_ERR_GROUP_LEAVE_GROUP_NOT_FOUND:
			Mgr_.HandleLeft (GroupNum_);
			break;
		case TOX_ERR_GROUP_LEAVE_TOO_LONG:
			RunLeave ({});
			break;
		case TOX_ERR_GROUP_LEAVE_FAIL_SEND:
			constexpr auto maxRetries = 10;
			if (retry < maxRetries)
			{
				using namespace std::chrono_literals;
				co_await (1s * (retry + 1));
				RunLeave (msg, retry + 1);
			}
			break;
		}
	}

	void GroupChatEntry::Leave (const QString& msg)
	{
		RunLeave (msg);
	}

	QString GroupChatEntry::GetNick () const
	{
		return Nick_;
	}

	Util::ContextTask<void> GroupChatEntry::RunSetNick (QString nick, int retry)
	{
		co_await Util::AddContextObject { *this };

		const auto tox = Mgr_.GetAccount ().GetTox ();
		if (!tox)
			co_return;

		const auto result = co_await tox->RunWithStrError (&tox_group_self_set_name, nick, GroupNum_);
		if (result.IsRight ())
		{
			Nick_ = nick;
			co_return;
		}

		qWarning () << "failed to change nickname:" << tox_err_group_self_name_set_to_string (result.GetLeft ());

		constexpr auto maxRetries = 10;
		if (result.GetLeft () == TOX_ERR_GROUP_SELF_NAME_SET_FAIL_SEND && retry < maxRetries)
		{
			using namespace std::chrono_literals;
			co_await (1s * (retry + 1));
			RunSetNick (Nick_, retry + 1);
		}
	}

	void GroupChatEntry::SetNick (const QString& nick)
	{
		RunSetNick (nick);
	}
}
