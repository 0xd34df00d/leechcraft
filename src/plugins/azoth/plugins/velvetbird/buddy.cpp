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

#include "buddy.h"
#include <QImage>
#include <QtDebug>
#include "account.h"
#include "util.h"
#include "convimmessage.h"

namespace LeechCraft
{
namespace Azoth
{
namespace VelvetBird
{
	Buddy::Buddy (PurpleBuddy *buddy, Account *account)
	: QObject (account)
	, Account_ (account)
	, Buddy_ (buddy)
	{
		Update ();
	}

	QObject* Buddy::GetObject ()
	{
		return this;
	}

	QObject* Buddy::GetParentAccount () const
	{
		return Account_;
	}

	ICLEntry::Features Buddy::GetEntryFeatures () const
	{
		return ICLEntry::FPermanentEntry | ICLEntry::FSupportsGrouping;
	}

	ICLEntry::EntryType Buddy::GetEntryType () const
	{
		return ICLEntry::ETChat;
	}

	QString Buddy::GetEntryName () const
	{
		return QString::fromUtf8 (purple_buddy_get_alias (Buddy_));
	}

	void Buddy::SetEntryName (const QString& name)
	{
		purple_blist_alias_buddy (Buddy_, name.toUtf8 ().constData ());
	}

	QString Buddy::GetEntryID () const
	{
		return Account_->GetAccountID () + GetHumanReadableID ();
	}

	QString Buddy::GetHumanReadableID () const
	{
		return QString::fromUtf8 (purple_buddy_get_name (Buddy_));
	}

	QStringList Buddy::Groups () const
	{
		return Group_.isEmpty () ? QStringList () : QStringList (Group_);
	}

	void Buddy::SetGroups (const QStringList& groups)
	{
		const auto& newGroup = groups.value (0);

		PurpleGroup *group = 0;
		if (!newGroup.isEmpty ())
		{
			const auto& utf8 = newGroup.toUtf8 ();
			group = purple_find_group (utf8.constData ());
			if (!group)
			{
				group = purple_group_new (utf8.constData ());
				purple_blist_add_group (group, nullptr);
			}
		}

		purple_blist_add_buddy (Buddy_, nullptr, group, nullptr);
	}

	QStringList Buddy::Variants () const
	{
		return QStringList ();
	}

	QObject* Buddy::CreateMessage (IMessage::MessageType, const QString&, const QString& body)
	{
		return new ConvIMMessage (body, IMessage::DOut, this);
	}

	QList<QObject*> Buddy::GetAllMessages () const
	{
		QList<QObject*> result;
		for (auto msg : Messages_)
			result << msg;
		return result;
	}

	void Buddy::PurgeMessages (const QDateTime& before)
	{
		// TODO
	}

	void Buddy::SetChatPartState (ChatPartState state, const QString& variant)
	{
	}

	EntryStatus Buddy::GetStatus (const QString& variant) const
	{
		return Status_;
	}

	QImage Buddy::GetAvatar () const
	{
		return QImage ();
	}

	QString Buddy::GetRawInfo () const
	{
		return QString ();
	}

	void Buddy::ShowInfo ()
	{
	}

	QList<QAction*> Buddy::GetActions () const
	{
		return QList<QAction*> ();
	}

	QMap<QString, QVariant> Buddy::GetClientInfo (const QString& variant) const
	{
		return QMap<QString, QVariant> ();
	}

	void Buddy::MarkMsgsRead ()
	{
	}

	void Buddy::Send (ConvIMMessage *msg)
	{
		Store (msg);

		if (!PurpleConv_)
		{
			PurpleConv_.reset (purple_conversation_new (PURPLE_CONV_TYPE_IM,
						Account_->GetPurpleAcc (),
						purple_buddy_get_name (Buddy_)),
					purple_conversation_destroy);
			PurpleConv_->ui_data = this;
			purple_conversation_set_logging (PurpleConv_.get (), false);
		}

		purple_conv_im_send (PurpleConv_->u.im, msg->GetBody ().toUtf8 ().constData ());
	}

	void Buddy::Store (ConvIMMessage *msg)
	{
		Messages_ << msg;
		emit gotMessage (msg);
	}

	void Buddy::SetConv (PurpleConversation *conv)
	{
		PurpleConv_.reset (conv, purple_conversation_destroy);
		PurpleConv_->ui_data = this;
	}

	void Buddy::HandleMessage (const char *who, const char *body, PurpleMessageFlags flags, time_t time)
	{
		if (flags & PURPLE_MESSAGE_SEND)
			return;

		auto msg = new ConvIMMessage (QString::fromUtf8 (body), IMessage::DIn, this);
		if (time)
			msg->SetDateTime (QDateTime::fromTime_t (time));
		Store (msg);
	}

	PurpleBuddy* Buddy::GetPurpleBuddy () const
	{
		return Buddy_;
	}

	void Buddy::Update ()
	{
		if (Name_ != GetEntryName ())
		{
			Name_ = GetEntryName ();
			emit nameChanged (Name_);
		}

		auto purpleStatus = purple_presence_get_active_status (Buddy_->presence);
		const auto& status = FromPurpleStatus (Account_->GetPurpleAcc (), purpleStatus);
		if (status != Status_)
		{
			Status_ = status;
			emit statusChanged (Status_, QString ());
		}

		auto groupNode = purple_buddy_get_group (Buddy_);
		const auto& newGroup = groupNode ? QString::fromUtf8 (groupNode->name) : QString ();
		if (newGroup != Group_)
		{
			Group_ = newGroup;
			emit groupsChanged ({ Group_ });
		}
	}
}
}
}
