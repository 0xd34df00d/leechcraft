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

#include "msnaccount.h"
#include <msn/notificationserver.h>
#include <util/util.h>
#include <interfaces/iproxyobject.h>
#include "msnprotocol.h"
#include "callbacks.h"
#include "core.h"
#include "msnaccountconfigwidget.h"
#include "zheetutil.h"
#include "msnbuddyentry.h"
#include "msnmessage.h"
#include "sbmanager.h"
#include "groupmanager.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Zheet
{
	MSNAccount::MSNAccount (const QString& name, MSNProtocol *parent)
	: QObject (parent)
	, Proto_ (parent)
	, Name_ (name)
	, Server_ ("messenger.hotmail.com")
	, Port_ (1863)
	, CB_ (new Callbacks (this))
	, Conn_ (0)
	, SB_ (new SBManager (CB_, this))
	, GroupManager_ (new GroupManager (CB_, this))
	, Connecting_ (false)
	{
		connect (CB_,
				SIGNAL (finishedConnecting ()),
				this,
				SLOT (handleConnected ()));
		connect (CB_,
				SIGNAL (gotBuddies (QList<MSN::Buddy*>)),
				this,
				SLOT (handleGotBuddies (QList<MSN::Buddy*>)));
		connect (CB_,
				SIGNAL (weChangedState (State)),
				this,
				SLOT (handleWeChangedState (State)));
		connect (CB_,
				SIGNAL (gotMessage (QString, MSN::Message*)),
				this,
				SLOT (handleGotMessage (QString, MSN::Message*)));
		connect (CB_,
				SIGNAL (buddyChangedStatus (QString, State)),
				this,
				SLOT (handleBuddyChangedStatus (QString, State)));
	}

	void MSNAccount::Init ()
	{
		const QString& pass = Core::Instance ().GetPluginProxy ()->GetAccountPassword (this);
		Conn_ = new MSN::NotificationServerConnection (Passport_,
				pass.toUtf8 ().constData (), *CB_);
		CB_->SetNotificationServerConnection (Conn_);
	}

	QByteArray MSNAccount::Serialize () const
	{
		quint16 version = 1;

		QByteArray result;
		{
			QDataStream ostr (&result, QIODevice::WriteOnly);
			ostr << version
				<< Name_
				<< ZheetUtil::FromStd (Passport_)
				<< Server_
				<< Port_;
		}

		return result;
	}

	MSNAccount* MSNAccount::Deserialize (const QByteArray& data, MSNProtocol *parent)
	{
		quint16 version = 0;

		QDataStream in (data);
		in >> version;

		if (version != 1)
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown version"
					<< version;
			return 0;
		}

		QString name;
		in >> name;

		QString passport;

		MSNAccount *result = new MSNAccount (name, parent);
		in >> passport
			>> result->Server_
			>> result->Port_;
		result->Passport_ = ZheetUtil::ToStd (passport);
		result->Init ();

		return result;
	}

	void MSNAccount::FillConfig (MSNAccountConfigWidget *w)
	{
		Passport_ = ZheetUtil::ToStd (w->GetID ());

		const QString& pass = w->GetPassword ();
		if (!pass.isEmpty ())
			Core::Instance ().GetPluginProxy ()->SetPassword (pass, this);
	}

	MSN::NotificationServerConnection* MSNAccount::GetNSConnection () const
	{
		return Conn_;
	}

	SBManager* MSNAccount::GetSBManager () const
	{
		return SB_;
	}

	GroupManager* MSNAccount::GetGroupManager () const
	{
		return GroupManager_;
	}

	MSNBuddyEntry* MSNAccount::GetBuddy (const QString& id) const
	{
		return Entries_ [id];
	}

	MSNBuddyEntry* MSNAccount::GetBuddyByCID (const QString& cid) const
	{
		return CID2Entry_ [cid];
	}

	QObject* MSNAccount::GetObject ()
	{
		return this;
	}

	QObject* MSNAccount::GetParentProtocol () const
	{
		return Proto_;
	}

	IAccount::AccountFeatures MSNAccount::GetAccountFeatures () const
	{
		return FRenamable | FSupportsXA | FHasConfigurationDialog;
	}

	QList<QObject*> MSNAccount::GetCLEntries ()
	{
		QList<QObject*> result;
		Q_FOREACH (auto obj, Entries_.values ())
			result << obj;
		return result;
	}

	QString MSNAccount::GetAccountName () const
	{
		return Name_;
	}

	QString MSNAccount::GetOurNick () const
	{
		return ZheetUtil::FromStd (Passport_);
	}

	void MSNAccount::RenameAccount (const QString& name)
	{
		Name_ = name;
	}

	QByteArray MSNAccount::GetAccountID () const
	{
		return "Azoth.msn.libmsn." + ZheetUtil::FromStd (Passport_).toUtf8 ();
	}

	QList<QAction*> MSNAccount::GetActions () const
	{
		return QList<QAction*> ();
	}

	void MSNAccount::QueryInfo (const QString&)
	{
	}

	void MSNAccount::OpenConfigurationDialog ()
	{
	}

	EntryStatus MSNAccount::GetState () const
	{
		return CurrentStatus_;
	}

	void MSNAccount::ChangeState (const EntryStatus& status)
	{
		if (!Conn_)
		{
			qWarning () << Q_FUNC_INFO
					<< "null Conn_";
			return;
		}

		if (status.State_ == SOffline)
			Conn_->disconnect ();
		else if (!Conn_->isConnected ())
		{
			if (!Connecting_)
			{
				Conn_->connect (ZheetUtil::ToStd (Server_), Port_);
				Connecting_ = true;
			}
			PendingStatus_ = status;
		}
		else
		{
			uint cid = 0;
			cid += MSN::MSNC1;
			cid += MSN::MSNC2;
			cid += MSN::MSNC3;
			cid += MSN::MSNC4;
			cid += MSN::MSNC5;
			cid += MSN::MSNC6;
			cid += MSN::MSNC7;
			cid += MSN::SupportMultiPacketMessaging;

			Conn_->setState (ZheetUtil::ToMSNState (status.State_), cid);

			MSN::personalInfo info;
			info.PSM = ZheetUtil::ToStd (status.StatusString_);
			Conn_->setPersonalStatus (info);
		}
	}

	void MSNAccount::Synchronize ()
	{
	}

	void MSNAccount::Authorize (QObject*)
	{
	}

	void MSNAccount::DenyAuth (QObject*)
	{
	}

	void MSNAccount::RequestAuth (const QString& entry, const QString&, const QString& name, const QStringList&)
	{
		const auto& id = ZheetUtil::ToStd (entry);
		Conn_->addToAddressBook (id, ZheetUtil::ToStd (name));
	}

	void MSNAccount::RemoveEntry (QObject *entryObj)
	{
		auto entry = qobject_cast<MSNBuddyEntry*> (entryObj);
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< "invalid entry"
					<< entryObj;
			return;
		}

		const auto& id = ZheetUtil::ToStd (entry->GetContactID ());
		const auto& pass = ZheetUtil::ToStd (entry->GetHumanReadableID ());
		Conn_->delFromAddressBook (id, pass);
	}

	QObject* MSNAccount::GetTransferManager () const
	{
		return 0;
	}

	void MSNAccount::handleConnected ()
	{
		Connecting_ = false;
		ChangeState (PendingStatus_);
	}

	void MSNAccount::handleWeChangedState (State st)
	{
		CurrentStatus_.State_ = st;
		emit statusChanged (CurrentStatus_);
	}

	void MSNAccount::handleBuddyChangedStatus (const QString& buddy, State st)
	{
		if (!Entries_.contains (buddy))
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown buddy"
					<< buddy;
			return;
		}

		Entries_ [buddy]->UpdateState (st);
	}

	void MSNAccount::handleGotBuddies (const QList<MSN::Buddy*>& buddies)
	{
		qDebug () << Q_FUNC_INFO;
		QList<QObject*> result;

		Q_FOREACH (const MSN::Buddy *buddy, buddies)
		{
			const auto& id = ZheetUtil::FromStd (buddy->userName);
			if (Entries_.contains (id))
				continue;

			auto entry = new MSNBuddyEntry (*buddy, this);

			result << entry;
			Entries_ [id] = entry;
			CID2Entry_ [entry->GetContactID ()] = entry;

			qDebug () << "buddy" << entry->GetEntryID () << entry->GetEntryName ();
		}

		emit gotCLItems (result);
	}

	void MSNAccount::handleGotMessage (const QString& from, MSN::Message *msnMsg)
	{
		if (!Entries_.contains (from))
		{
			qWarning () << Q_FUNC_INFO
					<< "got a message from unknown buddy"
					<< from;
			return;
		}

		auto entry = Entries_ [from];
		MSNMessage *msg = new MSNMessage (msnMsg, entry);
		entry->HandleMessage (msg);
	}
}
}
}
