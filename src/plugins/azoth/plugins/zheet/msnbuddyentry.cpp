/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#include "msnbuddyentry.h"
#include <QImage>
#include <msn/notificationserver.h>
#include <interfaces/iproxyobject.h>
#include "msnaccount.h"
#include "zheetutil.h"
#include "core.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Zheet
{
	MSNBuddyEntry::MSNBuddyEntry (const MSN::Buddy& buddy, MSNAccount *acc)
	: QObject (acc)
	, Account_ (acc)
	, Buddy_ (buddy)
	{
		Q_FOREACH (auto grp, buddy.groups)
			Groups_ << *grp;
	}

	QObject* MSNBuddyEntry::GetObject ()
	{
		return this;
	}

	QObject* MSNBuddyEntry::GetParentAccount () const
	{
		return Account_;
	}

	ICLEntry::Features MSNBuddyEntry::GetEntryFeatures () const
	{
		return FPermanentEntry |
				//FSupportsAuth |
				FSupportsGrouping;
	}

	ICLEntry::EntryType MSNBuddyEntry::GetEntryType () const
	{
		return ETChat;
	}

	QString MSNBuddyEntry::GetEntryName () const
	{
		return ZheetUtil::FromStd (Buddy_.friendlyName);
	}

	void MSNBuddyEntry::SetEntryName (const QString&)
	{
	}

	QString MSNBuddyEntry::GetEntryID () const
	{
		return Account_->GetAccountID () + "_" + ZheetUtil::FromStd (Buddy_.userName);
	}

	QStringList MSNBuddyEntry::Groups () const
	{
		QStringList result;
		Q_FOREACH (MSN::Group grp, Groups_)
			result << ZheetUtil::FromStd (grp.name);
		return result;
	}

	void MSNBuddyEntry::SetGroups (const QStringList& groups)
	{
	}

	QStringList MSNBuddyEntry::Variants () const
	{
		return QStringList (QString ());
	}

	QObject* MSNBuddyEntry::CreateMessage (IMessage::MessageType type, const QString&, const QString& body)
	{
		return 0;
	}

	QList<QObject*> MSNBuddyEntry::GetAllMessages () const
	{
		return QList<QObject*> ();
	}

	void MSNBuddyEntry::PurgeMessages (const QDateTime& before)
	{
	}

	void MSNBuddyEntry::SetChatPartState (ChatPartState, const QString&)
	{

	}

	EntryStatus MSNBuddyEntry::GetStatus (const QString&) const
	{
		return EntryStatus ();
	}

	QImage MSNBuddyEntry::GetAvatar () const
	{
		return QImage ();
	}

	QString MSNBuddyEntry::GetRawInfo () const
	{
		return QString ();
	}

	void MSNBuddyEntry::ShowInfo ()
	{
	}

	QList<QAction*> MSNBuddyEntry::GetActions () const
	{
		return QList<QAction*> ();
	}

	QMap<QString, QVariant> MSNBuddyEntry::GetClientInfo (const QString&) const
	{
		return QMap<QString, QVariant> ();
	}

	void MSNBuddyEntry::MarkMsgsRead ()
	{
	}
}
}
}
