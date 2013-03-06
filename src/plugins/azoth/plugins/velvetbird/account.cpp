/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "account.h"
#include <QtDebug>
#include <QTimer>
#include <util/passutils.h>
#include <util/util.h>
#include <interfaces/core/ientitymanager.h>
#include "protocol.h"
#include "util.h"
#include "buddy.h"

namespace LeechCraft
{
namespace Azoth
{
namespace VelvetBird
{
	Account::Account (const QString& name, PurpleAccount *acc, Protocol *proto)
	: QObject (proto)
	, Name_ (name)
	, Account_ (acc)
	, Proto_ (proto)
	{
		UpdateStatus ();
	}

	PurpleAccount* Account::GetPurpleAcc () const
	{
		return Account_;
	}

	QObject* Account::GetObject ()
	{
		return this;
	}

	QObject* Account::GetParentProtocol () const
	{
		return Proto_;
	}

	IAccount::AccountFeatures Account::GetAccountFeatures () const
	{
		return FRenamable;
	}

	QList<QObject*> Account::GetCLEntries ()
	{
		QList<QObject*> result;
		for (auto buddy : Buddies_)
			result << buddy;
		return result;
	}

	QString Account::GetAccountName () const
	{
		return Name_;
	}

	QString Account::GetOurNick () const
	{
		return QString::fromUtf8 (purple_account_get_name_for_display (Account_));
	}

	void Account::RenameAccount (const QString& name)
	{
		Name_ = name;
		emit accountRenamed (name);
	}

	QByteArray Account::GetAccountID () const
	{
		return Proto_->GetProtocolID () + "_" + purple_account_get_username (Account_);
	}

	QList<QAction*> Account::GetActions () const
	{
		return {};
	}

	void Account::QueryInfo (const QString&)
	{
	}

	void Account::OpenConfigurationDialog ()
	{
	}

	EntryStatus Account::GetState () const
	{
		return CurrentStatus_;
	}

	void Account::ChangeState (const EntryStatus& status)
	{
		if (status.State_ == SOffline)
		{
			if (!purple_account_is_disconnected (Account_))
				purple_account_disconnect (Account_);
			purple_account_set_enabled (Account_, "leechcraft.azoth", false);
			CurrentStatus_ = status;
			emit statusChanged (CurrentStatus_);
			return;
		}

		if (!purple_account_get_password (Account_))
		{
			const auto& str = Util::GetPassword ("Azoth." + GetAccountID (),
					tr ("Enter password for account %1:").arg (GetAccountName ()), Proto_, true);
			if (str.isEmpty ())
				return;

			purple_account_set_password (Account_, str.toUtf8 ().constData ());
		}

		if (!purple_account_get_enabled (Account_, "leechcraft.azoth"))
			purple_account_set_enabled (Account_, "leechcraft.azoth", true);

		auto type = purple_account_get_status_type_with_primitive (Account_, ToPurpleState (status.State_));
		auto statusId = type ? purple_status_type_get_id (type) : "available";
		purple_account_set_status (Account_,
				statusId,
				true,
				"message",
				status.StatusString_.toUtf8 ().constData (),
				NULL);
	}

	void Account::Authorize (QObject*)
	{
	}

	void Account::DenyAuth (QObject*)
	{
	}

	void Account::RequestAuth (const QString& entry,
			const QString& msg, const QString& name, const QStringList&)
	{
		auto buddy = purple_buddy_new (Account_,
				entry.toUtf8 ().constData (),
				name.isEmpty () ? nullptr : name.toUtf8 ().constData ());
		purple_blist_add_buddy (buddy, NULL, NULL, NULL);
		purple_account_add_buddy_with_invite (Account_, buddy, msg.toUtf8 ().constData ());
	}

	void Account::RemoveEntry (QObject *entryObj)
	{
		auto buddy = qobject_cast<Buddy*> (entryObj);
		purple_account_remove_buddy (Account_, buddy->GetPurpleBuddy (), NULL);
		purple_blist_remove_buddy (buddy->GetPurpleBuddy ());
	}

	QObject* Account::GetTransferManager () const
	{
		return 0;
	}

	void Account::UpdateBuddy (PurpleBuddy *purpleBuddy)
	{
		if (!Buddies_.contains (purpleBuddy))
		{
			auto buddy = new Buddy (purpleBuddy, this);
			Buddies_ [purpleBuddy] = buddy;
			emit gotCLItems ({ buddy });
		}

		Buddies_ [purpleBuddy]->Update ();
	}

	void Account::RemoveBuddy (PurpleBuddy *purpleBuddy)
	{
		auto buddy = Buddies_.take (purpleBuddy);
		if (!buddy)
			return;

		emit removedCLItems ({ buddy });
		buddy->deleteLater ();
	}

	void Account::HandleDisconnect (PurpleConnectionError error, const char *text)
	{
		const auto& prevStatus = GetState ();

		CurrentStatus_ = EntryStatus ();
		emit statusChanged (CurrentStatus_);

		const auto& eNotify = Util::MakeNotification (GetAccountName (),
				tr ("Connection error: %1.")
					.arg (QString::fromUtf8 (text)),
				PCritical_);
		Proto_->GetCoreProxy ()->GetEntityManager ()->HandleEntity (eNotify);

		if (error == PURPLE_CONNECTION_ERROR_AUTHENTICATION_FAILED)
			QMetaObject::invokeMethod (this,
					"handleAuthFailure",
					Qt::QueuedConnection,
					Q_ARG (LeechCraft::Azoth::EntryStatus, prevStatus));
	}

	void Account::HandleConvLessMessage (PurpleConversation *conv,
			const char *who, const char *message, PurpleMessageFlags flags, time_t mtime)
	{
		for (auto buddy : Buddies_)
		{
			if (buddy->GetHumanReadableID () != who)
				continue;

			buddy->SetConv (conv);
			buddy->HandleMessage (who, message, flags, mtime);
			break;
		}
	}

	void Account::UpdateStatus ()
	{
		HandleStatus (purple_account_get_active_status (Account_));
	}

	void Account::HandleStatus (PurpleStatus *status)
	{
		CurrentStatus_ = status ? FromPurpleStatus (Account_, status) : EntryStatus ();
		qDebug () << Q_FUNC_INFO << CurrentStatus_.State_;
		emit statusChanged (CurrentStatus_);

		QTimer::singleShot (5000, this, SLOT (updateIcon ()));
	}

	void Account::updateIcon ()
	{
		auto img = purple_buddy_icons_find_account_icon (Account_);
		if (!img)
			return;

		const auto data = purple_imgstore_get_data (img);
		const auto size = purple_imgstore_get_size (img);

		auto image = QImage::fromData (reinterpret_cast<const uchar*> (data), size);
		qDebug () << Q_FUNC_INFO << image.isNull ();

		purple_imgstore_unref (img);
	}

	void Account::handleAuthFailure (const EntryStatus& prevStatus)
	{
		ChangeState (EntryStatus ());
		const auto& str = Util::GetPassword ("Azoth." + GetAccountID (),
				tr ("Enter password for account %1:").arg (GetAccountName ()), Proto_, false);
		if (str.isEmpty ())
			return;

		purple_account_set_password (Account_, str.toUtf8 ().constData ());

		ChangeState (prevStatus);

		HandleStatus (purple_account_get_active_status (Account_));
	}
}
}
}
