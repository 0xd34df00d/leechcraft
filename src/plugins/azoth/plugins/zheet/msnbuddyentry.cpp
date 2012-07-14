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

#include "msnbuddyentry.h"
#include <algorithm>
#include <QImage>
#include <msn/notificationserver.h>
#include <interfaces/azoth/iproxyobject.h>
#include <interfaces/azoth/azothutil.h>
#include "msnaccount.h"
#include "msnmessage.h"
#include "zheetutil.h"
#include "core.h"
#include "groupmanager.h"
#include "sbmanager.h"

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
			Groups_ << ZheetUtil::FromStd (grp->name);

		qDebug () << Q_FUNC_INFO << Groups_;
		std::for_each (buddy.properties.cbegin (), buddy.properties.cend (),
				[] (decltype (*buddy.properties.cbegin ()) item) { qDebug () << item.first.c_str () << ": " << item.second.c_str (); });

		try
		{
			ContactID_ = ZheetUtil::FromStd (buddy.properties.at ("contactId"));
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to get contact ID for"
					<< GetHumanReadableID ()
					<< e.what ();
			throw;
		}
	}

	void MSNBuddyEntry::HandleMessage (MSNMessage *msg)
	{
		AllMessages_ << msg;
		emit gotMessage (msg);
	}

	void MSNBuddyEntry::HandleNudge ()
	{
		emit attentionDrawn (QString (), QString ());
	}

	void MSNBuddyEntry::UpdateState (State st)
	{
		const auto& oldVars = Variants ();
		Status_.State_ = st;

		if (oldVars != Variants ())
			emit availableVariantsChanged (Variants ());
		emit statusChanged (Status_, QString ());
	}

	void MSNBuddyEntry::AddGroup (const QString& group)
	{
		if (Groups_.contains (group))
			return;

		Groups_ << group;
		emit groupsChanged (Groups_);
	}

	void MSNBuddyEntry::RemoveGroup (const QString& group)
	{
		if (Groups_.removeOne (group))
			emit groupsChanged (Groups_);
	}

	QString MSNBuddyEntry::GetContactID () const
	{
		return ContactID_;
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
		QString res = ZheetUtil::FromStd (Buddy_.friendlyName);
		if (res.isEmpty ())
			res = GetHumanReadableID ();
		return res;
	}

	void MSNBuddyEntry::SetEntryName (const QString&)
	{
	}

	QString MSNBuddyEntry::GetEntryID () const
	{
		return Account_->GetAccountID () + "_" + GetHumanReadableID ();
	}

	QString MSNBuddyEntry::GetHumanReadableID () const
	{
		return ZheetUtil::FromStd (Buddy_.userName);
	}

	QStringList MSNBuddyEntry::Groups () const
	{
		return Groups_;
	}

	void MSNBuddyEntry::SetGroups (const QStringList& groups)
	{
		Account_->GetGroupManager ()->SetGroups (this, groups, Groups ());
	}

	QStringList MSNBuddyEntry::Variants () const
	{
		return Status_.State_ == SOffline ?
				QStringList () :
				QStringList (QString ());
	}

	QObject* MSNBuddyEntry::CreateMessage (IMessage::MessageType type, const QString&, const QString& body)
	{
		MSNMessage *msg = new MSNMessage (IMessage::DOut, type, this);
		msg->SetBody (body);
		return msg;
	}

	QList<QObject*> MSNBuddyEntry::GetAllMessages () const
	{
		QList<QObject*> result;
		Q_FOREACH (auto msg, AllMessages_)
			result << msg;
		return result;
	}

	void MSNBuddyEntry::PurgeMessages (const QDateTime& before)
	{
		Azoth::Util::StandardPurgeMessages (AllMessages_, before);
	}

	void MSNBuddyEntry::SetChatPartState (ChatPartState, const QString&)
	{
	}

	EntryStatus MSNBuddyEntry::GetStatus (const QString&) const
	{
		return Status_;
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

	IAdvancedCLEntry::AdvancedFeatures MSNBuddyEntry::GetAdvancedFeatures () const
	{
		return AFSupportsAttention;
	}

	void MSNBuddyEntry::DrawAttention (const QString& text, const QString&)
	{
		Account_->GetSBManager ()->SendNudge (text, this);
	}
}
}
}
