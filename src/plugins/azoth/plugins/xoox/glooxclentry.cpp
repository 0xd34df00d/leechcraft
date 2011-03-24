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

#include "glooxclentry.h"
#include <boost/bind.hpp>
#include <QStringList>
#include <QtDebug>
#include <QXmppClient.h>
#include <QXmppRosterManager.h>
#include <interfaces/iaccount.h>
#include <interfaces/azothcommon.h>
#include "glooxaccount.h"
#include "core.h"
#include "clientconnection.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	GlooxCLEntry::GlooxCLEntry (const QString& jid, GlooxAccount *parent)
	: EntryBase (parent)
	, BareJID_ (jid)
	{
	}

	GlooxCLEntry::GlooxCLEntry (GlooxCLEntry::OfflineDataSource_ptr ods, GlooxAccount *parent)
	: EntryBase (parent)
	, ODS_ (ods)
	{
		const QString& pre = Account_->GetAccountID () + '_';
		if (ods->ID_.startsWith (pre))
			BareJID_ = ods->ID_.mid (pre.size ());
		else
		{
			qWarning () << Q_FUNC_INFO
					<< "ODS's ID doesn't start with"
					<< pre
					<< ods->ID_;
			BareJID_ = ods->ID_;
		}
	}

	GlooxCLEntry::OfflineDataSource_ptr GlooxCLEntry::ToOfflineDataSource () const
	{
		if (ODS_)
			return ODS_;

		OfflineDataSource_ptr ods (new OfflineDataSource);
		ods->ID_ = GetEntryID ();
		ods->Name_ = GetEntryName ();
		ods->Groups_ = Groups ();
		ods->AuthStatus_ = GetAuthStatus ();

		return ods;
	}

	void GlooxCLEntry::Convert2ODS ()
	{
		ODS_ = ToOfflineDataSource ();
		CurrentStatus_.clear ();
		emit availableVariantsChanged (QStringList ());
		emit statusChanged (EntryStatus (SOffline, QString ()), QString ());
	}
	
	QString GlooxCLEntry::JIDFromID (GlooxAccount *acc, const QString& id)
	{
		const QString& pre = acc->GetAccountID () + '_';
		return id.startsWith (pre) ?
				id.mid (pre.size ()) :
				id;
	}

	void GlooxCLEntry::UpdateRI (const QXmppRosterIq::Item& item)
	{
		ODS_.reset ();

		emit availableVariantsChanged (Variants ());
		emit nameChanged (GetEntryName ());
		emit groupsChanged (item.groups ().toList ());
	}

	QXmppRosterIq::Item GlooxCLEntry::GetRI () const
	{
		return Account_->GetClientConnection ()->GetClient ()->
				rosterManager ().getRosterEntry (BareJID_);
	}

	QObject* GlooxCLEntry::GetParentAccount () const
	{
		return Account_;
	}

	ICLEntry::Features GlooxCLEntry::GetEntryFeatures () const
	{
		return FPermanentEntry |
				FSupportsRenames |
				FSupportsAuth |
				FSupportsGrouping;
	}

	ICLEntry::EntryType GlooxCLEntry::GetEntryType () const
	{
		return ETChat;
	}

	QString GlooxCLEntry::GetEntryName () const
	{
		if (ODS_)
			return ODS_->Name_;

		QString name = GetRI ().name ();
		if (name.isEmpty ())
			return BareJID_;
		return name;
	}

	void GlooxCLEntry::SetEntryName (const QString& name)
	{
		if (ODS_)
			return;

		QXmppRosterIq::Item item = GetRI ();
		item.setName (name);
		Account_->GetClientConnection ()->Update (item);

		emit nameChanged (name);
	}

	QString GlooxCLEntry::GetEntryID () const
	{
		if (ODS_)
			return ODS_->ID_;

		return Account_->GetAccountID () + '_' + BareJID_;
	}
	
	QString GlooxCLEntry::GetHumanReadableID() const
	{
		return BareJID_;
	}

	QStringList GlooxCLEntry::Groups () const
	{
		if (ODS_)
			return ODS_->Groups_;

		return GetRI ().groups ().toList ();
	}

	void GlooxCLEntry::SetGroups (const QStringList& groups)
	{
		if (ODS_)
			return;

		QXmppRosterIq::Item item = GetRI ();
		item.setGroups (QSet<QString>::fromList (groups));
		Account_->GetClientConnection ()->Update (item);
	}

	QStringList GlooxCLEntry::Variants () const
	{
		QStringList result;

		if (!ODS_)
		{
			const QMap<QString, QXmppPresence>& presences =
					Account_->GetClientConnection ()->GetClient ()->
							rosterManager ().getAllPresencesForBareJid (BareJID_);
			if (presences.size () == 1)
				result << presences.begin ().key ();
			else
			{
				QMap<int, QList<QString> > prio2res;
				for (QMap<QString, QXmppPresence>::const_iterator i = presences.begin ();
						i != presences.end (); ++i)
					prio2res [i->status ().priority ()] << i.key ();
				Q_FOREACH (int prio, prio2res.keys ())
					result << prio2res [prio];
				std::reverse (result.begin (), result.end ());
			}
		}

		return result;
	}
	
	EntryStatus GlooxCLEntry::GetStatus (const QString& variant) const
	{
		if (ODS_)
			return EntryStatus ();

		QXmppRosterManager& rm = Account_->
				GetClientConnection ()->GetClient ()->rosterManager ();
		if (!rm.isRosterReceived ())
			return EntryBase::GetStatus (variant);

		const QMap<QString, QXmppPresence>& press = rm.getAllPresencesForBareJid (GetJID ());
		if (!press.size ())
			return EntryBase::GetStatus (variant);

		QXmppPresence max = press.begin ().value ();
		Q_FOREACH (const QString& resource, press.keys ())
		{
			if (!variant.isEmpty () && variant == resource)
			{
				max = press [resource];
				break;
			}
			const QXmppPresence& pres = press [resource];
			if (pres.status ().priority () > max.status ().priority ())
				max = pres;
		}
		return EntryStatus (static_cast<State> (max.status ().type ()),
				max.status ().statusText ());
	}

	QObject* GlooxCLEntry::CreateMessage (IMessage::MessageType type,
			const QString& variant, const QString& text)
	{
		if (ODS_)
		{
			// TODO
			return 0;
		}

		QObject *msg = Account_->CreateMessage (type, variant, text, GetRI ());
		AllMessages_ << msg;
		return msg;
	}

	AuthStatus GlooxCLEntry::GetAuthStatus () const
	{
		if (ODS_)
			return ODS_->AuthStatus_;

		return static_cast<AuthStatus> (GetRI ().subscriptionType ());
	}

	void GlooxCLEntry::RevokeAuth (const QString& reason)
	{
		if (ODS_)
			return;

		Account_->GetClientConnection ()->RevokeSubscription (GetJID (), reason);
	}

	void GlooxCLEntry::Unsubscribe (const QString& reason)
	{
		if (ODS_)
			return;

		Account_->GetClientConnection ()->Unsubscribe (GetJID (), reason);;
	}

	void GlooxCLEntry::RerequestAuth (const QString& reason)
	{
		if (ODS_)
			return;

		Account_->GetClientConnection ()->Subscribe (GetJID (),
				reason,
				GetEntryName (),
				Groups ());
	}

	QString GlooxCLEntry::GetJID () const
	{
		return BareJID_;
	}
}
}
}
