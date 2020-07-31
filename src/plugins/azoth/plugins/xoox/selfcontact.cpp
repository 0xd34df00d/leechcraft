/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "selfcontact.h"
#include <algorithm>
#include <QXmppVCardManager.h>
#include "clientconnection.h"
#include "vcarddialog.h"
#include "accountsettingsholder.h"
#include "glooxmessage.h"

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	SelfContact::SelfContact (const QString& fullJid, GlooxAccount *acc)
	: EntryBase (acc->GetSettings ()->GetJID (), acc)
	{
		UpdateJID (fullJid);

		connect (this,
				SIGNAL (vcardUpdated ()),
				this,
				SLOT (handleSelfVCardUpdated ()));
	}

	ICLEntry::Features SelfContact::GetEntryFeatures () const
	{
		return FSupportsGrouping | FPermanentEntry | FSelfContact;
	}

	ICLEntry::EntryType SelfContact::GetEntryType () const
	{
		return EntryType::Chat;
	}

	QString SelfContact::GetEntryName () const
	{
		return Account_->GetNick ();
	}

	void SelfContact::SetEntryName (const QString&)
	{
		qWarning () << Q_FUNC_INFO
				<< "can't set name of self contact";
	}

	QString SelfContact::GetEntryID () const
	{
		return Account_->GetAccountID () + '_' + ".self";
	}

	QStringList SelfContact::Groups () const
	{
		return QStringList (tr ("Self contact"));
	}

	void SelfContact::SetGroups (const QStringList&)
	{
		qWarning () << Q_FUNC_INFO
				<< "can't set groups of self contact";
	}

	QStringList SelfContact::Variants () const
	{
		auto result = Status2Prio_.keys ();
		std::sort (result.begin (), result.end (),
				[this] (const QString& left, const QString& right)
					{ return Status2Prio_ [left] > Status2Prio_ [right]; });
		return result;
	}

	EntryStatus SelfContact::GetStatus (const QString& resource) const
	{
		if (resource == Resource_)
			return Account_->GetState ();

		return EntryBase::GetStatus (resource);
	}

	IMessage* SelfContact::CreateMessage (IMessage::Type type,
			const QString& variant, const QString& text)
	{
		const auto msg = Account_->CreateMessage (type, variant, text, GetJID ());
		AllMessages_ << msg;
		return msg;
	}

	QList<QAction*> SelfContact::GetActions () const
	{
		return EntryBase::GetActions ();
	}

	void SelfContact::HandlePresence (const QXmppPresence& pres, const QString& resource)
	{
		EntryBase::HandlePresence (pres, resource);

		if (pres.type () == QXmppPresence::Available)
			UpdatePriority (resource, pres.priority ());
		else
			RemoveVariant (resource, Account_->GetClientConnection ()->GetOurResource () == resource);
	}

	void SelfContact::UpdatePriority (const QString& resource, int prio)
	{
		Status2Prio_ [resource] = prio;
		emit availableVariantsChanged (Variants ());
	}

	void SelfContact::RemoveVariant (const QString& resource, bool thisInstance)
	{
		if (thisInstance)
			for (const auto& otherResource : Status2Prio_.keys ())
				if (otherResource != resource)
					RemoveVariant (otherResource, false);

		Status2Prio_.remove (resource);
		Variants_.remove (resource);

		EntryBase::SetStatus (EntryStatus (SOffline, QString ()),
				resource,
				QXmppPresence (QXmppPresence::Unavailable));
	}

	QString SelfContact::GetJID () const
	{
		return BareJID_;
	}

	void SelfContact::UpdateJID (const QString& fullJid)
	{
		std::tie (BareJID_, Resource_) = ClientConnection::Split (fullJid);

		emit availableVariantsChanged (Variants ());
	}

	void SelfContact::handleSelfVCardUpdated ()
	{
		Account_->UpdateOurPhotoHash (VCardPhotoHash_);
	}
}
}
}
