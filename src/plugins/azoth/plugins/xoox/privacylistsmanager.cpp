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

#include "privacylistsmanager.h"
#include <QDomElement>
#include <QXmppClient.h>

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	const QString NsPrivacy = "jabber:iq:privacy";
	
	PrivacyListItem::PrivacyListItem (const QString& value,
			PrivacyListItem::Type type, PrivacyListItem::Action action)
	: Value_ (value)
	, Type_ (type)
	, Action_ (action)
	, Stanzas_ (STAll)
	{
	}
	
	void PrivacyListItem::Parse (const QDomElement& itemElem)
	{
		const QString& type = itemElem.attribute ("type");
		if (type == "jid")
			Type_ = TJid;
		else if (type == "subscription")
			Type_ = TSubscription;
		else if (type == "group")
			Type_ = TGroup;
		else
			Type_ = TNone;
		
		Value_ = itemElem.attribute ("value");
		Action_ = itemElem.attribute ("action") == "deny" ?
				ADeny :
				AAllow;

		Stanzas_ = STNone;
		if (!itemElem.firstChildElement ("message").isNull ())
			Stanzas_ |= STMessage;
		if (!itemElem.firstChildElement ("presence-in").isNull ())
			Stanzas_ |= STPresenceIn;
		if (!itemElem.firstChildElement ("presence-out").isNull ())
			Stanzas_ |= STPresenceOut;
		if (!itemElem.firstChildElement ("iq").isNull ())
			Stanzas_ |= STIq;
		
		if (Stanzas_ == STNone)
			Stanzas_ = STAll;
	}
	
	QXmppElement PrivacyListItem::ToXML() const
	{
		QXmppElement item;
		item.setTagName ("item");

		switch (Type_)
		{
		case TJid:
			item.setAttribute ("type", "jid");
			break;
		case TSubscription:
			item.setAttribute ("type", "subscription");
			break;
		case TGroup:
			item.setAttribute ("type", "group");
			break;
		case TNone:
			break;
		}
		
		item.setAttribute ("action", Action_ == ADeny ? "deny" : "allow");

		if (!Value_.isEmpty ())
			item.setAttribute ("value", Value_);
		
		if (Stanzas_ != STAll)
		{
			if (Stanzas_ & STMessage)
			{
				QXmppElement elem;
				elem.setTagName ("message");
				item.appendChild (elem);
			}
			if (Stanzas_ & STPresenceIn)
			{
				QXmppElement elem;
				elem.setTagName ("presence-in");
				item.appendChild (elem);
			}
			if (Stanzas_ &STPresenceOut)
			{
				QXmppElement elem;
				elem.setTagName ("presence-out");
				item.appendChild (elem);
			}
			if (Stanzas_ & STIq)
			{
				QXmppElement elem;
				elem.setTagName ("iq");
				item.appendChild (elem);
			}
		}
		
		return item;
	}
	
	PrivacyListItem::Type PrivacyListItem::GetType () const
	{
		return Type_;
	}
	
	void PrivacyListItem::SetType (PrivacyListItem::Type type)
	{
		Type_ = type;
	}
	
	PrivacyListItem::Action PrivacyListItem::GetAction () const
	{
		return Action_;
	}
	
	void PrivacyListItem::SetAction (PrivacyListItem::Action action)
	{
		Action_ = action;
	}
	
	QString PrivacyListItem::GetValue () const
	{
		return Value_;
	}
	
	void PrivacyListItem::SetValue (const QString& value)
	{
		Value_ = value;
	}
	
	PrivacyListItem::StanzaTypes PrivacyListItem::GetStanzaTypes () const
	{
		return Stanzas_;
	}
	
	void PrivacyListItem::SetStanzaTypes (PrivacyListItem::StanzaTypes st)
	{
		Stanzas_ = st;
	}
	
	PrivacyList::PrivacyList (const QString& name)
	: Name_ (name)
	{
	}
	
	void PrivacyList::Parse (const QDomElement& list)
	{
		Name_ = list.attribute ("name");
		
		QMap<int, PrivacyListItem> items;

		QDomElement itemElem = list.firstChildElement ("item");
		while (!itemElem.isNull ())
		{
			PrivacyListItem item;
			item.Parse (itemElem);
			items [itemElem.attribute ("order").toInt ()] = item;

			itemElem = itemElem.nextSiblingElement ("item");
		}
		
		SetItems (items.values ());
	}
	
	QXmppElement PrivacyList::ToXML () const
	{
		QXmppElement listElem;
		listElem.setTagName ("list");
		listElem.setAttribute ("name", Name_);
		
		int i = 1;
		Q_FOREACH (const PrivacyListItem& item, Items_)
		{
			QXmppElement itemElem = item.ToXML ();
			itemElem.setAttribute ("order", QString::number (i++));
			listElem.appendChild (itemElem);
		}
		
		return listElem;
	}
	
	QString PrivacyList::GetName () const
	{
		return Name_;
	}
	
	void PrivacyList::SetName (const QString& name)
	{
		Name_ = name;
	}
	
	QList<PrivacyListItem> PrivacyList::GetItems () const
	{
		return Items_;
	}
	
	void PrivacyList::SetItems (const QList<PrivacyListItem>& items)
	{
		Items_ = items;
	}
	
	void PrivacyListsManager::QueryLists ()
	{
		QXmppElement query;
		query.setTagName ("query");
		query.setAttribute ("xmlns", NsPrivacy);
		
		QXmppIq iq;
		iq.setExtensions (query);
		
		ID2Type_ [iq.id ()] = QTQueryLists;
		
		client ()->sendPacket (iq);
	}
	
	void PrivacyListsManager::QueryList (const QString& name)
	{
		QXmppElement list;
		list.setTagName ("list");
		list.setAttribute ("name", name);

		QXmppElement query;
		query.setTagName ("query");
		query.setAttribute ("xmlns", NsPrivacy);
		query.appendChild (list);
		
		QXmppIq iq;
		iq.setExtensions (query);
		
		ID2Type_ [iq.id ()] = QTGetList;
		
		client ()->sendPacket (iq);
	}
	
	void PrivacyListsManager::ActivateList (const QString& name, ListType type)
	{
		QXmppElement list;
		list.setTagName (type == LTActive ? "active" : "default");
		if (!name.isEmpty ())
			list.setAttribute ("name", name);

		QXmppElement query;
		query.setTagName ("query");
		query.setAttribute ("xmlns", NsPrivacy);
		query.appendChild (list);
		
		QXmppIq iq (QXmppIq::Set);
		iq.setExtensions (query);
		
		client ()->sendPacket (iq);
	}
	
	void PrivacyListsManager::SetList (const PrivacyList& list)
	{
		QXmppElement query;
		query.setTagName ("query");
		query.setAttribute ("xmlns", NsPrivacy);
		query.appendChild (list.ToXML ());
		
		QXmppIq iq (QXmppIq::Set);
		iq.setExtensions (query);
		
		client ()->sendPacket (iq);
	}

	QStringList PrivacyListsManager::discoveryFeatures () const
	{
		return QStringList (NsPrivacy);
	}
	
	bool PrivacyListsManager::handleStanza (const QDomElement& elem)
	{
		if (elem.tagName () != "iq")
			return false;
		
		if (elem.attribute ("type") == "set" &&
				elem.firstChildElement ("query").namespaceURI () == NsPrivacy)
		{
			QXmppIq iq (QXmppIq::Result);
			iq.setId (elem.attribute ("id"));
			client ()->sendPacket (iq);
			return true;
		}
		
		if (!ID2Type_.contains (elem.attribute ("id")))
			return false;
		
		switch (ID2Type_ [elem.attribute ("id")])
		{
		case QTQueryLists:
			HandleListQueryResult (elem);
			break;
		case QTGetList:
			HandleList (elem);
			break;
		}
		
		return true;
	}
	
	void PrivacyListsManager::HandleListQueryResult (const QDomElement& elem)
	{
		const QDomElement& query = elem.firstChildElement ("query");
		const QString& active = query.firstChildElement ("active").attribute ("name");
		const QString& def = query.firstChildElement ("default").attribute ("name");
		
		QStringList lists;
		QDomElement listElem = query.firstChildElement ("list");
		while (!listElem.isNull ())
		{
			lists << listElem.attribute ("name");
			listElem = listElem.nextSiblingElement ("list");
		}
		
		emit gotLists (lists, active, def);
	}
	
	void PrivacyListsManager::HandleList (const QDomElement& elem)
	{
		const QDomElement& query = elem.firstChildElement ("query");

		PrivacyList list;
		list.Parse (query.firstChildElement ("list"));
		
		emit gotList (list);
	}
}
}
}
