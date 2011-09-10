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

#include "riexmanager.h"
#include <QDomElement>
#include <QXmppClient.h>
#include <QXmppMessage.h>
#include "core.h"
#include "capsdatabase.h"
#include "entrybase.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	const QString NsRIEX = "http://jabber.org/protocol/rosterx";

	RIEXManager::Item::Item ()
	: Action_ (AAdd)
	{
	}

	RIEXManager::Item::Item (RIEXManager::Item::Action action,
			QString jid, QString name, QStringList groups)
	: Action_ (action)
	, JID_ (jid)
	, Name_ (name)
	, Groups_ (groups)
	{
	}

	RIEXManager::Item::Action RIEXManager::Item::GetAction () const
	{
		return Action_;
	}

	void RIEXManager::Item::SetAction (RIEXManager::Item::Action action)
	{
		Action_ = action;
	}

	QString RIEXManager::Item::GetJID () const
	{
		return JID_;
	}

	void RIEXManager::Item::SetJID (QString jid)
	{
		JID_ = jid;
	}

	QString RIEXManager::Item::GetName () const
	{
		return Name_;
	}

	void RIEXManager::Item::SetName (QString name)
	{
		Name_ = name;
	}

	QStringList RIEXManager::Item::GetGroups () const
	{
		return Groups_;
	}

	void RIEXManager::Item::SetGroups (QStringList groups)
	{
		Groups_ = groups;
	}

	QStringList RIEXManager::discoveryFeatures () const
	{
		return QStringList (NsRIEX);
	}

	bool RIEXManager::handleStanza (const QDomElement& elem)
	{
		if ((elem.tagName () != "message" && elem.tagName () != "iq") ||
				elem.attribute ("from").isEmpty ())
			return false;

		const QDomElement& x = elem.firstChildElement ("x");
		if (x.namespaceURI () != NsRIEX)
			return false;

		const bool isIq = elem.tagName () == "iq";

		QList<Item> items;

		QDomElement item = x.firstChildElement ("item");
		while (!item.isNull ())
		{
			QStringList groups;

			QDomElement group = item.firstChildElement ("group");
			while (!group.isNull ())
			{
				groups << group.text ();
				group = group.nextSiblingElement ("group");
			}

			Item::Action act = Item::AAdd;
			const QString& actAttr = item.attribute ("action");
			if (actAttr == "modify")
				act = Item::AModify;
			else if (actAttr == "delete")
				act = Item::ADelete;

			items << Item (act, item.attribute ("jid"), item.attribute ("name"), groups);

			item = item.nextSiblingElement ("item");
		}

		emit gotItems (elem.attribute ("from"), items, !isIq);

		return isIq;
	}

	void RIEXManager::SuggestItems (EntryBase *to, QList<RIEXManager::Item> items, QString message)
	{
		QXmppElement x;
		x.setTagName ("x");
		x.setAttribute ("xmlns", NsRIEX);

		Q_FOREACH (const RIEXManager::Item& item, items)
		{
			QXmppElement itElem;
			itElem.setTagName ("item");

			if (!item.GetJID ().isEmpty ())
				itElem.setAttribute ("jid", item.GetJID ());

			if (!item.GetName ().isEmpty ())
				itElem.setAttribute ("name", item.GetName ());

			switch (item.GetAction ())
			{
			case Item::AModify:
				itElem.setAttribute ("action", "modify");
				break;
			case Item::ADelete:
				itElem.setAttribute ("action", "delete");
				break;
			default:
				itElem.setAttribute ("action", "add");
				break;
			}

			Q_FOREACH (const QString& group, item.GetGroups ())
			{
				QXmppElement groupElem;
				groupElem.setTagName ("group");
				groupElem.setValue (group);

				itElem.appendChild (groupElem);
			}

			x.appendChild (itElem);
		}

		QString suppRes;

		CapsDatabase *db = Core::Instance ().GetCapsDatabase ();
		Q_FOREACH (const QString& variant, to->Variants ())
		{
			const QByteArray& ver = to->GetVariantVerString (variant);
			const QStringList& features = db->Get (ver);
			if (features.contains (NsRIEX))
			{
				suppRes = variant;
				break;
			}
		}

		if (!suppRes.isEmpty ())
		{
			QXmppIq iq (QXmppIq::Set);
			iq.setTo (to->GetJID () + '/' + suppRes);
			iq.setExtensions (x);
			client ()->sendPacket (iq);
		}
		else
		{
			QXmppMessage msg;
			msg.setTo (to->GetHumanReadableID ());
			msg.setBody (message);
			msg.setExtensions (x);
			client ()->sendPacket (msg);
		}
	}
}
}
}
