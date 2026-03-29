/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "glooxclentry.h"
#include <algorithm>
#include <QStringList>
#include <QAction>
#include <QtDebug>
#include <QXmppClient.h>
#include <QXmppRosterManager.h>
#include <util/util.h>
#include <util/sll/containerconversions.h>
#include <util/sll/qtutil.h>
#include <interfaces/azoth/iaccount.h>
#include <interfaces/azoth/azothcommon.h>
#include <interfaces/azoth/iproxyobject.h>
#include "glooxaccount.h"
#include "core.h"
#include "clientconnection.h"
#include "clientconnectionextensionsmanager.h"
#include "gwoptionsdialog.h"
#include "xeps/privacylistsmanager.h"
#include "glooxmessage.h"
#include "util.h"

namespace LC::Azoth::Xoox
{
	GlooxCLEntry::GlooxCLEntry (const QString& jid, GlooxAccount *parent)
	: EntryBase (jid, parent)
	{
	}

	QString GlooxCLEntry::JIDFromID (GlooxAccount *acc, const QString& id)
	{
		const QString& pre = acc->GetAccountID () + '_';
		return id.startsWith (pre) ?
				id.mid (pre.size ()) :
				id;
	}

	bool GlooxCLEntry::IsRosterEntry () const
	{
		return IsRosterEntry_;
	}

	void GlooxCLEntry::PromoteToRosterEntry ()
	{
		IsRosterEntry_ = true;
	}

	void GlooxCLEntry::UpdateRI (const QXmppRosterIq::Item& item)
	{
		emit Emitter_.availableVariantsChanged (Variants ());
		emit Emitter_.nameChanged (GetEntryName ());
		emit Emitter_.groupsChanged (item.groups ().values ());
	}

	QXmppRosterIq::Item GlooxCLEntry::GetRI () const
	{
		return Account_->GetClientConnection ()->Exts ().Get<QXmppRosterManager> ().getRosterEntry (HumanReadableId_);
	}

	ICLEntry::Features GlooxCLEntry::GetEntryFeatures () const
	{
		auto result = FSupportsAuth | FSupportsGrouping;
		result |= IsRosterEntry_ ?
				FSupportsRenames | FPermanentEntry :
				FSessionEntry;
		return result;
	}

	ICLEntry::EntryType GlooxCLEntry::GetEntryType () const
	{
		return EntryType::Chat;
	}

	QString GlooxCLEntry::GetEntryName () const
	{
		QString name = GetRI ().name ();
		if (name.isEmpty ())
			return HumanReadableId_;
		return name;
	}

	void GlooxCLEntry::SetEntryName (const QString& name)
	{
		QXmppRosterIq::Item item = GetRI ();
		item.setName (name);
		Account_->GetClientConnection ()->Update (item);

		// TODO emit delayed, when we get the ack of any sort
		emit Emitter_.nameChanged (name);
	}

	QString GlooxCLEntry::GetEntryID () const
	{
		return Account_->GetAccountID () + '_' + HumanReadableId_;
	}

	QStringList GlooxCLEntry::Groups () const
	{
		QStringList groups = GetRI ().groups ().values ();
		if (AuthRequested_)
			groups += tr ("Unauthorized users");
		return groups;
	}

	void GlooxCLEntry::SetGroups (const QStringList& groups)
	{
		QXmppRosterIq::Item item = GetRI ();
		item.setGroups (Util::AsSet (groups));
		Account_->GetClientConnection ()->Update (item);
	}

	QStringList GlooxCLEntry::Variants () const
	{
		QStringList result;

		const auto& presences = Account_->GetClientConnection ()->Exts ()
				.Get<QXmppRosterManager> ().getAllPresencesForBareJid (HumanReadableId_);
		if (presences.size () == 1)
			result << presences.begin ().key ();
		else
		{
			QMap<int, QList<QString>> prio2res;
			for (const auto& pair : Util::Stlize (presences))
				prio2res [pair.second.priority ()] << pair.first;
			for (const auto& list : prio2res)
				result += list;
			std::reverse (result.begin (), result.end ());
		}

		return result;
	}

	EntryStatus GlooxCLEntry::GetStatus (const QString& variant) const
	{
		if (AuthRequested_)
			return EntryStatus (SOnline, QString ());

		auto& rm = Account_->GetClientConnection ()->Exts ().Get<QXmppRosterManager> ();
		if (!rm.isRosterReceived ())
			return EntryBase::GetStatus (variant);

		const auto& press = rm.getAllPresencesForBareJid (GetJID ());
		if (!press.size ())
			return EntryBase::GetStatus (variant);

		QXmppPresence max = press.begin ().value ();
		for (const auto& [resource, pres] : Util::Stlize (press))
		{
			if (!variant.isEmpty () && variant == resource)
			{
				max = pres;
				break;
			}
			if (pres.priority () > max.priority ())
				max = pres;
		}
		return EntryStatus (static_cast<State> (max.availableStatusType () + 1),
				max.statusText ());
	}

	void GlooxCLEntry::SendMessage (const OutgoingMessage& message)
	{
		const auto msg = new GlooxMessage (message, GetJID (), Account_->GetClientConnection ().get ());
		AllMessages_ << msg;
		Account_->SendMessage (*msg);
		emit Emitter_.gotMessage (msg);
	}

	QList<QAction*> GlooxCLEntry::GetActions () const
	{
		auto result = EntryBase::GetActions ();
		QString gvVar;

		if (IsGateway (&gvVar))
		{
			if (GWActions_.isEmpty ())
			{
				auto login = new QAction (tr ("Login"), Account_);
				login->setProperty ("Azoth/Xoox/Variant", gvVar);
				connect (login,
						SIGNAL (triggered ()),
						this,
						SLOT (handleGWLogin ()));
				GWActions_ << login;

				auto logout = new QAction (tr ("Logout"), Account_);
				logout->setProperty ("Azoth/Xoox/Variant", gvVar);
				connect (logout,
						SIGNAL (triggered ()),
						this,
						SLOT (handleGWLogout ()));
				GWActions_ << logout;

				auto edit = new QAction (tr ("Gateway preferences..."), Account_);
				edit->setProperty ("Azoth/Xoox/Variant", gvVar);
				edit->setProperty ("ActionIcon", "preferences-other");
				connect (edit,
						SIGNAL (triggered ()),
						this,
						SLOT (handleGWEdit ()));
				GWActions_ << edit;

				GWActions_ << Util::CreateSeparator (Account_);
			}
		}
		else if (!GWActions_.isEmpty ())
			GWActions_.clear ();

		result += GWActions_;

		return result;
	}

	bool GlooxCLEntry::IsGateway (QString *variant) const
	{
		for (const auto& [varCand, info] : Util::Stlize (Variants_))
			for (const auto& id : info.Identities_)
				if (id.category () == "gateway")
				{
					if (variant)
						*variant = varCand;
					return true;
				}

		return false;
	}

	AuthStatus GlooxCLEntry::GetAuthStatus () const
	{
		return static_cast<AuthStatus> (GetRI ().subscriptionType ());
	}

	void GlooxCLEntry::ResendAuth (const QString& reason)
	{
		SetAuthRequested (false);
		RerequestAuth (QString ());
		Account_->GetClientConnection ()->GrantSubscription (GetJID (), reason);
	}

	void GlooxCLEntry::RevokeAuth (const QString& reason)
	{
		SetAuthRequested (false);
		Account_->GetClientConnection ()->RevokeSubscription (GetJID (), reason);
	}

	void GlooxCLEntry::Unsubscribe (const QString& reason)
	{
		Account_->GetClientConnection ()->Unsubscribe (GetJID (), reason);;
	}

	void GlooxCLEntry::RerequestAuth (const QString& reason)
	{
		Account_->GetClientConnection ()->Subscribe (GetJID (),
				reason,
				GetEntryName (),
				Groups ());
	}

	QString GlooxCLEntry::GetJID () const
	{
		return HumanReadableId_;
	}

	void GlooxCLEntry::SetAuthRequested (bool auth)
	{
		AuthRequested_ = auth;
		emit Emitter_.statusChanged (GetStatus (QString ()), QString ());
		emit Emitter_.groupsChanged (Groups ());
	}

	void GlooxCLEntry::SendGWPresence (QXmppPresence::Type type)
	{
		const auto& variant = sender ()->property ("Azoth/Xoox/Variant").toString ();
		QString jid = GetJID ();
		if (!variant.isEmpty ())
			jid += '/' + variant;

		QXmppPresence avail (type);
		avail.setTo (jid);
		Account_->GetClientConnection ()->GetClient ()->sendPacket (avail);
	}

	void GlooxCLEntry::handleGWLogin ()
	{
		SendGWPresence (QXmppPresence::Available);
	}

	void GlooxCLEntry::handleGWLogout ()
	{
		SendGWPresence (QXmppPresence::Unavailable);
	}

	void GlooxCLEntry::handleGWEdit ()
	{
		auto dia = new GWOptionsDialog (Account_->GetClientConnection ()->GetClient (), GetJID ());
		dia->setAttribute (Qt::WA_DeleteOnClose);
		dia->show ();
	}
}
