/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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
#include "account.h"

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
		return ICLEntry::FPermanentEntry;
	}

	ICLEntry::EntryType Buddy::GetEntryType () const
	{
		return ICLEntry::ETChat;
	}

	QString Buddy::GetEntryName () const
	{
		return QString::fromUtf8 (purple_buddy_get_alias (Buddy_));
	}

	void Buddy::SetEntryName (const QString&)
	{
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
		return QStringList ();
	}

	void Buddy::SetGroups (const QStringList& groups)
	{
	}

	QStringList Buddy::Variants () const
	{
		return QStringList ();
	}

	QObject* Buddy::CreateMessage (IMessage::MessageType type, const QString& variant, const QString& body)
	{
		return 0;
	}

	QList<QObject*> Buddy::GetAllMessages () const
	{
		return QList<QObject*> ();
	}

	void Buddy::PurgeMessages (const QDateTime& before)
	{
	}

	void Buddy::SetChatPartState (ChatPartState state, const QString& variant)
	{
	}

	EntryStatus Buddy::GetStatus (const QString& variant) const
	{
		return EntryStatus ();
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

	void Buddy::Update ()
	{
	}
}
}
}
