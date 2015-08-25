/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 **********************************************************************/

#include "glooxclentry.h"
#include <algorithm>
#include <QStringList>
#include <QAction>
#include <QtDebug>
#include <QXmppClient.h>
#include <QXmppRosterManager.h>
#include <util/util.h>
#include <util/sll/qtutil.h>
#include <interfaces/azoth/iaccount.h>
#include <interfaces/azoth/azothcommon.h>
#include <interfaces/azoth/iproxyobject.h>
#include "glooxaccount.h"
#include "core.h"
#include "clientconnection.h"
#include "capsmanager.h"
#include "gwoptionsdialog.h"
#include "privacylistsmanager.h"
#include "glooxmessage.h"
#include "vcardstorage.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	void Save (OfflineDataSource_ptr ods, QXmlStreamWriter *w, IProxyObject *proxy)
	{
		w->writeStartElement ("entry");
			w->writeTextElement ("id", ods->ID_.toUtf8 ().toPercentEncoding ("@"));
			w->writeTextElement ("name", ods->Name_);
			w->writeTextElement ("authstatus", proxy->AuthStatusToString (ods->AuthStatus_));

			w->writeStartElement ("groups");
			Q_FOREACH (const QString& group, ods->Groups_)
				w->writeTextElement ("group", group);
			w->writeEndElement ();
		w->writeEndElement ();
	}

	namespace
	{
		QString GetBareJID (const QString& entryId, GlooxAccount * const acc)
		{
			const auto& pre = acc->GetAccountID () + '_';
			if (!entryId.startsWith (pre))
			{
				qWarning () << Q_FUNC_INFO
						<< "entry ID doesn't start with"
						<< pre
						<< entryId;
				return entryId;
			}

			return entryId.mid (pre.size ());
		}

		void LoadVCard (const QDomElement& vcardElem,
				const QString& entryId, GlooxAccount * const acc, VCardStorage *storage)
		{
			if (vcardElem.isNull ())
				return;

			storage->SetVCard (GetBareJID (entryId, acc),
					QByteArray::fromBase64 (vcardElem.text ().toLatin1 ()));
		}
	}

	void Load (OfflineDataSource_ptr ods,
			const QDomElement& entry,
			IProxyObject *proxy,
			GlooxAccount * const acc)
	{
		const QByteArray& entryID = QByteArray::fromPercentEncoding (entry
					.firstChildElement ("id").text ().toLatin1 ());
		const QString& name = entry.firstChildElement ("name").text ();

		const auto& vcardData = entry.firstChildElement ("vcard").text ().toLatin1 ();
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

		const auto& authStatusText = entry.firstChildElement ("authstatus").text ();
		ods->AuthStatus_ = proxy->AuthStatusFromString (authStatusText);

		ods->VCardIq_.parse (vcardDoc.documentElement ());
		LoadVCard (entry.firstChildElement ("vcard"), entryID,
				acc, acc->GetParentProtocol ()->GetVCardStorage ());
	}

	GlooxCLEntry::GlooxCLEntry (const QString& jid, GlooxAccount *parent)
	: EntryBase (parent)
	, BareJID_ (jid)
	{
		Initialize ();
	}

	GlooxCLEntry::GlooxCLEntry (OfflineDataSource_ptr ods, GlooxAccount *parent)
	: EntryBase (parent)
	, BareJID_ (GetBareJID (ods->ID_, parent))
	, ODS_ (ods)
	{
		Initialize ();
	}

	void GlooxCLEntry::Initialize ()
	{
		BlockContact_ = new QAction (tr ("Block contact"), this);
		BlockContact_->setToolTip (tr ("Add this user to the active privacy list "
					"(not all servers support this feature)."));
		BlockContact_->setProperty ("ActionIcon", "im-ban-user");
		BlockContact_->setCheckable (true);
		connect (BlockContact_,
				SIGNAL (triggered (bool)),
				this,
				SLOT (addToPrivacyList (bool)));

		auto lists = Account_->GetClientConnection ()->GetPrivacyListsManager ();
		connect (lists,
				SIGNAL (currentListFetched (PrivacyList)),
				this,
				SLOT (checkIsBlocked (PrivacyList)));
		const auto& current = lists->GetCurrentList ();
		if (!current.GetItems ().isEmpty ())
			checkIsBlocked (current);
	}

	OfflineDataSource_ptr GlooxCLEntry::ToOfflineDataSource () const
	{
		if (ODS_)
			return ODS_;

		const auto ods = std::make_shared<OfflineDataSource> ();
		ods->ID_ = GetEntryID ();
		ods->Name_ = GetEntryName ();
		ods->Groups_ = Groups ();
		ods->AuthStatus_ = GetAuthStatus ();
		ods->VCardIq_ = GetVCard ();

		return ods;
	}

	void GlooxCLEntry::Convert2ODS ()
	{
		const auto& prevVariants = Variants ();

		ODS_ = ToOfflineDataSource ();
		CurrentStatus_.clear ();
		if (prevVariants.isEmpty ())
			return;

		emit availableVariantsChanged (QStringList ());
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

	IAccount* GlooxCLEntry::GetParentAccount () const
	{
		return Account_;
	}

	ICLEntry::Features GlooxCLEntry::GetEntryFeatures () const
	{
		ICLEntry::Features result = FSupportsAuth | FSupportsGrouping;

		auto& rm = Account_->GetClientConnection ()->GetClient ()->rosterManager ();
		const bool isPerm = ODS_ || rm.getRosterBareJids ().contains (GetJID ());

		result |= isPerm ?
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
			const auto& presences = Account_->GetClientConnection ()->GetClient ()->
							rosterManager ().getAllPresencesForBareJid (BareJID_);
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
			if (pres.priority () > max.priority ())
				max = pres;
		}
		return EntryStatus (static_cast<State> (max.availableStatusType () + 1),
				max.statusText ());
	}

	IMessage* GlooxCLEntry::CreateMessage (IMessage::Type type,
			const QString& variant, const QString& text)
	{
		if (ODS_)
			return nullptr;

		const auto msg = Account_->CreateMessage (type, variant, text, GetJID ());
		AllMessages_ << msg;
		return msg;
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
		result += BlockContact_;

		return result;
	}

	bool GlooxCLEntry::IsGateway (QString *variant) const
	{
		for (const QString& varCand : Variant2Identities_.keys ())
			for (const auto& id : Variant2Identities_ [varCand])
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

	void GlooxCLEntry::checkIsBlocked (const PrivacyList& list)
	{
		const auto& items = list.GetItems ();

		const auto& jid = GetJID ();
		const auto pos = std::find_if (items.begin (), items.end (),
				[&jid] (const PrivacyListItem& item)
				{
					return item.GetType () == PrivacyListItem::Type::Jid &&
							item.GetAction () == PrivacyListItem::Action::Deny &&
							item.GetValue () == jid;
				});
		BlockContact_->setChecked (pos != items.end ());
	}

	void GlooxCLEntry::addToPrivacyList (bool add)
	{
		const auto& jid = GetJID ();

		auto lists = Account_->GetClientConnection ()->GetPrivacyListsManager ();

		auto current = lists->GetCurrentList ();
		auto items = current.GetItems ();
		for (const auto& item : items)
		{
			if (item.GetType () != PrivacyListItem::Type::Jid)
				continue;

			if (item.GetValue () != jid)
				continue;

			if ((item.GetAction () == PrivacyListItem::Action::Allow && !add) ||
				(item.GetAction () == PrivacyListItem::Action::Deny && add))
				return;

			if (!add)
			{
				items.removeAll (item);
				break;
			}
		}
		if (add)
			items.append ({ jid, PrivacyListItem::Type::Jid });

		if (items.size () == current.GetItems ().size ())
			return;

		current.SetItems (items);
		lists->SetList (current);
	}
}
}
}
