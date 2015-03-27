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

#include "privacylistsmanager.h"
#include <QDomElement>
#include <QXmppClient.h>
#include <util/sll/prelude.h>
#include <util/sll/functional.h>
#include "clientconnectionerrormgr.h"
#include "clientconnection.h"
#include "serverinfostorage.h"

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
			Type_ = Type::Jid;
		else if (type == "subscription")
			Type_ = Type::Subscription;
		else if (type == "group")
			Type_ = Type::Group;
		else
			Type_ = Type::None;

		Value_ = itemElem.attribute ("value");
		Action_ = itemElem.attribute ("action") == "deny" ?
				Action::Deny :
				Action::Allow;

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
		case Type::Jid:
			item.setAttribute ("type", "jid");
			break;
		case Type::Subscription:
			item.setAttribute ("type", "subscription");
			break;
		case Type::Group:
			item.setAttribute ("type", "group");
			break;
		case Type::None:
			break;
		}

		item.setAttribute ("action", Action_ == Action::Deny ? "deny" : "allow");

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

	bool operator== (const PrivacyListItem& left, const PrivacyListItem& right)
	{
		return left.GetAction () == right.GetAction () &&
				left.GetType() == right.GetType () &&
				left.GetStanzaTypes () == right.GetStanzaTypes () &&
				left.GetValue () == right.GetValue ();
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
		for (const auto& item : Items_)
		{
			auto itemElem = item.ToXML ();
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

	const QList<PrivacyListItem>& PrivacyList::GetItems () const
	{
		return Items_;
	}

	void PrivacyList::SetItems (const QList<PrivacyListItem>& items)
	{
		Items_ = items;
	}

	PrivacyListsManager::PrivacyListsManager (ClientConnection *conn)
	: Conn_ { conn }
	{
	}

	bool PrivacyListsManager::IsSupported () const
	{
		const auto serverStorage = Conn_->GetServerInfoStorage ();
		if (!serverStorage->HasServerFeatures ())
			return true;

		return serverStorage->GetServerFeatures ().contains (NsPrivacy);
	}

	void PrivacyListsManager::QueryLists ()
	{
		QueryLists ({
				[this] (const QXmppIq& iq) { HandleListQueryError (iq); },
				Util::BindMemFn (&PrivacyListsManager::gotLists, this)
			});
	}

	void PrivacyListsManager::QueryLists (const QueryListsCont_f& cont)
	{
		QXmppElement query;
		query.setTagName ("query");
		query.setAttribute ("xmlns", NsPrivacy);

		QXmppIq iq;
		iq.setExtensions ({ query });

		const auto& id = iq.id ();
		ID2Type_ [id] = QueryType::QueryLists;
		QueryLists2Handler_ [id] = cont;

		client ()->sendPacket (iq);
	}

	void PrivacyListsManager::QueryList (const QString& name)
	{
		QueryList (name,
				{
					[] (const QXmppIq&) {},
					Util::BindMemFn (&PrivacyListsManager::gotList, this)
				});
	}

	void PrivacyListsManager::QueryList (const QString& name, const QueryListCont_f& cont)
	{
		QXmppElement list;
		list.setTagName ("list");
		list.setAttribute ("name", name);

		QXmppElement query;
		query.setTagName ("query");
		query.setAttribute ("xmlns", NsPrivacy);
		query.appendChild (list);

		QXmppIq iq;
		iq.setExtensions ({ query });

		const auto& id = iq.id ();
		ID2Type_ [id] = QueryType::GetList;
		QueryList2Handler_ [id] = cont;

		client ()->sendPacket (iq);
	}

	void PrivacyListsManager::ActivateList (const QString& name, ListType type)
	{
		QXmppElement list;
		list.setTagName (type == ListType::Active ? "active" : "default");
		if (!name.isEmpty ())
			list.setAttribute ("name", name);

		QXmppElement query;
		query.setTagName ("query");
		query.setAttribute ("xmlns", NsPrivacy);
		query.appendChild (list);

		QXmppIq iq (QXmppIq::Set);
		iq.setExtensions ({ query });

		client ()->sendPacket (iq);

		CurrentName_ = name;
		QueryList (name);
	}

	void PrivacyListsManager::SetList (const PrivacyList& list)
	{
		QXmppElement query;
		query.setTagName ("query");
		query.setAttribute ("xmlns", NsPrivacy);
		query.appendChild (list.ToXML ());

		QXmppIq iq (QXmppIq::Set);
		iq.setExtensions ({ query });

		client ()->sendPacket (iq);

		if (list.GetName () == CurrentName_)
		{
			CurrentList_ = list;
			emit currentListFetched (list);
		}
	}

	const PrivacyList& PrivacyListsManager::GetCurrentList () const
	{
		return CurrentList_;
	}

	QStringList PrivacyListsManager::discoveryFeatures () const
	{
		return { NsPrivacy };
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

		switch (ID2Type_.take (elem.attribute ("id")))
		{
		case QueryType::QueryLists:
			HandleListQueryResult (elem);
			break;
		case QueryType::GetList:
			HandleList (elem);
			break;
		}

		return true;
	}

	void PrivacyListsManager::HandleListQueryError (const QXmppIq& iq)
	{
		const auto& error = iq.error ();
		qWarning () << Q_FUNC_INFO
				<< "cannot fetch lists:"
				<< error.code ()
				<< error.condition ()
				<< error.text ();

		auto text = tr ("Cannot fetch lists.") +
				ClientConnectionErrorMgr::HandleErrorCondition (error.condition ());
		if (!error.text ().isEmpty ())
			text += " " + error.text ();
		emit listError (text);
	}

	void PrivacyListsManager::HandleListQueryResult (const QDomElement& elem)
	{
		const auto& id = elem.attribute ("id");

		const auto& handler = QueryLists2Handler_.take (id);

		if (elem.attribute ("type") == "error")
		{
			QXmppIq iq;
			iq.parse (elem);
			handler.Left (iq);
			return;
		}

		const auto& query = elem.firstChildElement ("query");
		const auto& active = query.firstChildElement ("active").attribute ("name");
		const auto& def = query.firstChildElement ("default").attribute ("name");

		QStringList lists;
		auto listElem = query.firstChildElement ("list");
		while (!listElem.isNull ())
		{
			lists << listElem.attribute ("name");
			listElem = listElem.nextSiblingElement ("list");
		}

		CurrentName_ = active.isEmpty () ? def : active;;
		QueryList (CurrentName_);

		handler.Right (lists, active, def);
	}

	void PrivacyListsManager::HandleList (const QDomElement& elem)
	{
		const auto& query = elem.firstChildElement ("query");

		PrivacyList list;
		list.Parse (query.firstChildElement ("list"));

		if (list.GetName () == CurrentName_)
		{
			CurrentList_ = list;
			currentListFetched (list);
		}

		QueryList2Handler_.take (elem.attribute ("id")).Right (list);
	}
}
}
}
