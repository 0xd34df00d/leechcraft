/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "groupjoinwidget.h"
#include <tox/tox.h>
#include <util/sll/qtutil.h>
#include <util/threads/coro.h>
#include "confsmanager.h"
#include "groupsmanager.h"
#include "toxaccount.h"

namespace LC::Azoth::Sarin
{
	GroupJoinWidget::GroupJoinWidget ()
	{
		Ui_.setupUi (this);
		Ui_.GroupId_->setMaxLength (TOX_GROUP_CHAT_ID_SIZE);
		Ui_.Nick_->setMaxLength (TOX_MAX_NAME_LENGTH);
		Ui_.Password_->setMaxLength (TOX_GROUP_MAX_PASSWORD_SIZE);

		connect (Ui_.GroupId_,
				&QLineEdit::textChanged,
				this,
				&GroupJoinWidget::CheckValidity);
		connect (Ui_.Nick_,
				&QLineEdit::textChanged,
				this,
				&GroupJoinWidget::CheckValidity);

		QTimer::singleShot (0, this, &GroupJoinWidget::CheckValidity);
	}

	void GroupJoinWidget::AccountSelected (QObject *accObj)
	{
		const auto acc = qobject_cast<ToxAccount*> (accObj);
		if (!acc)
		{
			qWarning () << "not a Tox account:" << accObj;
			return;
		}

		if (Ui_.Nick_->text ().isEmpty ())
			Ui_.Nick_->setText (acc->GetOurNick ());
	}

	namespace Constants
	{
		const auto Kind = "Kind"_qs;

		const auto Group = "Group"_qs;
		const auto GroupId = "GroupId"_qs;
		const auto Nick = "Nick"_qs;
		const auto Password = "Password"_qs;

		const auto Conf = "Conf"_qs;
		const auto Cookie = "Cookie"_qs;
		const auto FriendNum = "FriendNum"_qs;
	}

	void GroupJoinWidget::Join (QObject *accObj)
	{
		const auto acc = qobject_cast<ToxAccount*> (accObj);
		if (!acc)
		{
			qWarning () << "not a Tox account:" << accObj;
			return;
		}

		if (IsJoiningGroup ())
			acc->GetGroupsManager ().Join (Ui_.GroupId_->text (), Ui_.Nick_->text (), Ui_.Password_->text ());
		else
		{
			if (!ConfIdent_)
			{
				qWarning () << "inconsistent GUI state";
				return;
			}
			const auto& cookie = ConfIdent_->value (Constants::Cookie).toByteArray ();
			const auto friendNum = ConfIdent_->value (Constants::FriendNum).value<uint32_t> ();
			acc->GetConfsManager ().Join (cookie, friendNum);
		}
	}

	void GroupJoinWidget::Cancel ()
	{
	}

	QVariantMap GroupJoinWidget::GetIdentifyingData () const
	{
		if (IsJoiningGroup ())
			return
			{
				{ Constants::Kind, Constants::Group },
				{ Constants::GroupId, Ui_.GroupId_->text () },
				{ Constants::Nick, Ui_.Nick_->text () },
				{ Constants::Password, Ui_.Password_->text () },
			};
		else
			return
			{
				{ Constants::Kind, Constants::Conf },
				{ Constants::GroupId, Ui_.GroupId_->text () },
				{ Constants::Nick, Ui_.Nick_->text () },
				{ Constants::Password, Ui_.Password_->text () },
			};
	}

	void GroupJoinWidget::SetIdentifyingData (const QVariantMap& data)
	{
		if (data [Constants::Kind] == Constants::Group)
		{
			Ui_.GroupId_->setText (data [Constants::GroupId].toString ());
			Ui_.Nick_->setText (data [Constants::Nick].toString ());
			Ui_.Password_->setText (data [Constants::Password].toString ());
			Ui_.TargetPages_->setCurrentWidget (Ui_.GroupPage_);

			ConfIdent_.reset ();
		}
		else
		{
			Ui_.InviteText_->setText (tr ("You have been invited to a conference. Do you want to accept the invitation?"));
			Ui_.TargetPages_->setCurrentWidget (Ui_.ConfPage_);

			ConfIdent_ = data;
		}
	}

	QVariantMap GroupJoinWidget::GetConfIdentifyingData (const QByteArray& cookie, uint32_t friendNum)
	{
		return
		{
			{ Constants::Kind, Constants::Conf },
			{ Constants::Cookie, cookie },
			{ Constants::FriendNum, friendNum },
		};
	}

	bool GroupJoinWidget::IsJoiningGroup () const
	{
		return Ui_.TargetPages_->currentWidget () == Ui_.GroupPage_;
	}

	void GroupJoinWidget::CheckValidity ()
	{
		const bool valid = Ui_.GroupId_->text ().size () == TOX_GROUP_CHAT_ID_SIZE &&
				!Ui_.Nick_->text ().isEmpty ();
		emit validityChanged (valid);
	}
}
