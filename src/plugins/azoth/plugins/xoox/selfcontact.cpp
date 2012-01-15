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

#include "selfcontact.h"
#include "clientconnection.h"
#include "vcarddialog.h"
#include <QXmppVCardManager.h>

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	SelfContact::SelfContact (const QString& fullJid, GlooxAccount *acc)
	: EntryBase (acc)
	, FullJID_ (fullJid)
	{
		UpdateJID (fullJid);
	}
	
	QObject* SelfContact::GetParentAccount () const
	{
		return Account_;
	}
	
	ICLEntry::Features SelfContact::GetEntryFeatures () const
	{
		return FSupportsGrouping | FPermanentEntry;
	}

	ICLEntry::EntryType SelfContact::GetEntryType () const
	{
		return ETChat;
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
	
	QString SelfContact::GetHumanReadableID () const
	{
		return Account_->GetJID ();
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
		return Prio2Status_.values ();
	}
	
	EntryStatus SelfContact::GetStatus (const QString& resource) const
	{
		if (resource == Resource_)
			return Account_->GetState ();

		return EntryBase::GetStatus (resource);
	}
	
	QObject* SelfContact::CreateMessage (IMessage::MessageType type,
			const QString& variant, const QString& text)
	{
		QObject *msg = Account_->CreateMessage (type, variant, text, GetJID ());
		AllMessages_ << msg;
		return msg;
	}
	
	QList<QAction*> SelfContact::GetActions () const
	{
		return EntryBase::GetActions ();
	}
	
	void SelfContact::UpdatePriority (const QString& resource, int prio)
	{
		Prio2Status_ [prio] = resource;
		emit availableVariantsChanged (Variants ());
	}

	void SelfContact::RemoveVariant (const QString& resource)
	{
		Prio2Status_.remove (Prio2Status_.key (resource));
		CurrentStatus_.remove (resource);
		emit availableVariantsChanged (Variants ());
	}
	
	QString SelfContact::GetJID () const
	{
		return BareJID_;
	}
	
	void SelfContact::UpdateJID (const QString& fullJid)
	{
		ClientConnection::Split (fullJid, &BareJID_, &Resource_);
		
		emit availableVariantsChanged (Variants ());
	}
}
}
}
