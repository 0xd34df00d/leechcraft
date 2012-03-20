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

#include "glooxclentry.h"
#include <algorithm>
#include <QStringList>
#include <QAction>
#include <QtDebug>
#include <QXmppClient.h>
#include <QXmppRosterManager.h>
#include <interfaces/iaccount.h>
#include <interfaces/azothcommon.h>
#include <interfaces/iproxyobject.h>
#include "glooxaccount.h"
#include "core.h"
#include "clientconnection.h"
#include "capsmanager.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	void Save (OfflineDataSource_ptr ods, QXmlStreamWriter *w)
	{
		w->writeStartElement ("entry");
			w->writeTextElement ("id", ods->ID_.toUtf8 ().toPercentEncoding ("@"));
			w->writeTextElement ("name", ods->Name_);
			w->writeTextElement ("authstatus",
					Core::Instance ().GetPluginProxy ()->
						AuthStatusToString (ods->AuthStatus_));

			w->writeStartElement ("groups");
			Q_FOREACH (const QString& group, ods->Groups_)
				w->writeTextElement ("group", group);
			w->writeEndElement ();
			
			QByteArray vcardData;
			{
				QXmlStreamWriter vcardWriter (&vcardData);
				ods->VCardIq_.toXml (&vcardWriter);
			}
			w->writeTextElement ("vcard", vcardData.toBase64 ());
		w->writeEndElement ();
	}
	
	void Load (OfflineDataSource_ptr ods, const QDomElement& entry)
	{
		const QByteArray& entryID = QByteArray::fromPercentEncoding (entry
					.firstChildElement ("id").text ().toLatin1 ());
		const QString& name = entry.firstChildElement ("name").text ();
				
		QByteArray vcardData = entry.firstChildElement ("vcard").text ().toAscii ();
		QDomDocument vcardDoc;
		vcardDoc.setContent (QByteArray::fromBase64 (vcardData));

		QStringList groups;
		QDomElement group = entry
				.firstChildElement ("groups")
				.firstChildElement ("group");
		while (!group.isNull ())
		{
			const QString& text = group.text ();
			if (!text.isEmpty ())
				groups << text;
			group = group.nextSiblingElement ("group");
		}
		
		ods->Name_ = name;
		ods->ID_ = QString::fromUtf8 (entryID.constData ());
		ods->Groups_ = groups;
		ods->AuthStatus_ = Core::Instance ().GetPluginProxy ()->
				AuthStatusFromString (entry.firstChildElement ("authstatus").text ());
		ods->VCardIq_.parse (vcardDoc.documentElement ());
	}

	GlooxCLEntry::GlooxCLEntry (const QString& jid, GlooxAccount *parent)
	: EntryBase (parent)
	, BareJID_ (jid)
	, AuthRequested_ (false)
	, GWLogin_ (0)
	, GWLogout_ (0)
	{
	}

	GlooxCLEntry::GlooxCLEntry (OfflineDataSource_ptr ods, GlooxAccount *parent)
	: EntryBase (parent)
	, ODS_ (ods)
	, AuthRequested_ (false)
	, GWLogin_ (0)
	, GWLogout_ (0)
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
		
		SetVCard (ods->VCardIq_);
	}

	OfflineDataSource_ptr GlooxCLEntry::ToOfflineDataSource () const
	{
		if (ODS_)
			return ODS_;

		OfflineDataSource_ptr ods (new OfflineDataSource);
		ods->ID_ = GetEntryID ();
		ods->Name_ = GetEntryName ();
		ods->Groups_ = Groups ();
		ods->AuthStatus_ = GetAuthStatus ();
		ods->VCardIq_ = GetVCard ();

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
		ICLEntry::Features result = FSupportsAuth |
				FSupportsGrouping;
		if (Account_->GetClientConnection ()->
				GetClient ()->rosterManager().getRosterBareJids ().contains (GetJID ()))
			result |= FSupportsRenames | FPermanentEntry;
		else
			result |= FSessionEntry;
		return result;
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

		QStringList groups = GetRI ().groups ().toList ();
		if (AuthRequested_)
			groups += tr ("Unauthorized users");
		return groups;
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
				QMap<int, QList<QString>> prio2res;
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
		
		if (AuthRequested_)
			return EntryStatus (SOnline, QString ());

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

		QObject *msg = Account_->CreateMessage (type, variant, text, GetJID ());
		AllMessages_ << msg;
		return msg;
	}
	
	QList<QAction*> GlooxCLEntry::GetActions () const
	{
		auto baseActs = EntryBase::GetActions ();
		QString gvVar;
		bool gwFound = false;
		Q_FOREACH (const QString& varCand, Variant2Identities_.keys ())
		{
			Q_FOREACH (const auto& id, Variant2Identities_ [varCand])
				if (id.category () == "gateway")
				{
					gwFound = true;
					gvVar = varCand;
					break;
				}

			if (gwFound)	
				break;
		}
			
		if (gwFound)
		{
			if (!GWLogin_ || !GWLogout_)
			{
				GWLogin_ = new QAction (tr ("Login"), Account_);
				GWLogin_->setProperty ("Azoth/Xoox/Variant", gvVar);
				connect (GWLogin_,
						SIGNAL (triggered ()),
						this,
						SLOT (handleGWLogin ()));
				GWLogout_ = new QAction (tr ("Logout"), Account_);
				GWLogout_->setProperty ("Azoth/Xoox/Variant", gvVar);
				connect (GWLogout_,
						SIGNAL (triggered ()),
						this,
						SLOT (handleGWLogout ()));
			}

			baseActs << GWLogin_ << GWLogout_;
		}
		else
		{
			delete GWLogin_;
			GWLogin_ = 0;
			delete GWLogout_;
			GWLogout_ = 0;
		}
		
		return baseActs;
	}

	AuthStatus GlooxCLEntry::GetAuthStatus () const
	{
		if (ODS_)
			return ODS_->AuthStatus_;

		return static_cast<AuthStatus> (GetRI ().subscriptionType ());
	}
	
	void GlooxCLEntry::ResendAuth (const QString& reason)
	{
		if (ODS_)
			return;

		SetAuthRequested (false);
		RerequestAuth (QString ());
		Account_->GetClientConnection ()->GrantSubscription (GetJID (), reason);
	}

	void GlooxCLEntry::RevokeAuth (const QString& reason)
	{
		if (ODS_)
			return;

		SetAuthRequested (false);
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
	
	void GlooxCLEntry::SetAuthRequested (bool auth)
	{
		AuthRequested_ = auth;
		emit statusChanged (GetStatus (QString ()), QString ());
		emit groupsChanged (Groups ());
	}

	void GlooxCLEntry::SendGWPresence (QXmppPresence::Type type)
	{
		const auto& variant = sender ()->
				property ("Azoth/Xoox/Variant").toString ();
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
}
}
}
