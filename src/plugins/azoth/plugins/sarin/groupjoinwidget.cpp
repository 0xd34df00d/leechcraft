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
#include "toxaccount.h"

namespace LC::Azoth::Sarin
{
	GroupJoinWidget::GroupJoinWidget ()
	{
		Ui_.setupUi (this);
		Ui_.GroupId_->setInputMask (QString { TOX_GROUP_CHAT_ID_SIZE, 'N' });
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

	void GroupJoinWidget::Join (QObject *accObj)
	{
		const auto acc = qobject_cast<ToxAccount*> (accObj);
		if (!acc)
		{
			qWarning () << "not a Tox account:" << accObj;
			return;
		}

		acc->JoinGroup (Ui_.GroupId_->text (), Ui_.Nick_->text (), Ui_.Password_->text ());
	}

	void GroupJoinWidget::Cancel ()
	{
	}

	namespace Constants
	{
		const auto GroupId = "GroupId"_qs;
		const auto Nick = "Nick"_qs;
		const auto Password = "Password"_qs;
	}

	QVariantMap GroupJoinWidget::GetIdentifyingData () const
	{
		return
		{
			{ Constants::GroupId, Ui_.GroupId_->text () },
			{ Constants::Nick, Ui_.Nick_->text () },
			{ Constants::Password, Ui_.Password_->text () },
		};
	}

	void GroupJoinWidget::SetIdentifyingData (const QVariantMap& data)
	{
		Ui_.GroupId_->setText (data [Constants::GroupId].toString ());
		Ui_.Nick_->setText (data [Constants::Nick].toString ());
		Ui_.Password_->setText (data [Constants::Password].toString ());
	}

	void GroupJoinWidget::CheckValidity ()
	{
		const bool valid = Ui_.GroupId_->hasAcceptableInput () &&
				!Ui_.Nick_->text ().isEmpty ();
		emit validityChanged (valid);
	}
}
