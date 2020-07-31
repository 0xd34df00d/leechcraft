/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "riexmanager.h"
#include <QDomElement>
#include <QXmppClient.h>
#include <QXmppMessage.h>
#include "capsdatabase.h"
#include "entrybase.h"

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	const QString NsRIEX = "http://jabber.org/protocol/rosterx";

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

		QList<RIEXItem> items;

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

			auto act = RIEXItem::AAdd;
			const QString& actAttr = item.attribute ("action");
			if (actAttr == "modify")
				act = RIEXItem::AModify;
			else if (actAttr == "delete")
				act = RIEXItem::ADelete;

			items << RIEXItem { act, item.attribute ("jid"), item.attribute ("name"), groups };

			item = item.nextSiblingElement ("item");
		}

		QString body;
		if (elem.tagName () == "message")
		{
			QXmppMessage msg;
			msg.parse (elem);
			body = msg.body ();
		}

		emit gotItems (elem.attribute ("from"), items, body);

		return true;
	}

	void RIEXManager::SuggestItems (EntryBase *to, QList<RIEXItem> items, QString message)
	{
		QXmppElement x;
		x.setTagName ("x");
		x.setAttribute ("xmlns", NsRIEX);

		for (const auto& item : items)
		{
			QXmppElement itElem;
			itElem.setTagName ("item");

			if (!item.ID_.isEmpty ())
				itElem.setAttribute ("jid", item.ID_);

			if (!item.Nick_.isEmpty ())
				itElem.setAttribute ("name", item.Nick_);

			switch (item.Action_)
			{
			case RIEXItem::AModify:
				itElem.setAttribute ("action", "modify");
				break;
			case RIEXItem::ADelete:
				itElem.setAttribute ("action", "delete");
				break;
			default:
				itElem.setAttribute ("action", "add");
				break;
			}

			for (const auto& group : item.Groups_)
			{
				QXmppElement groupElem;
				groupElem.setTagName ("group");
				groupElem.setValue (group);

				itElem.appendChild (groupElem);
			}

			x.appendChild (itElem);
		}

		QString suppRes;

		const auto capsDb = to->GetParentAccount ()->GetParentProtocol ()->GetCapsDatabase ();

		for (const auto& variant : to->Variants ())
		{
			const QByteArray& ver = to->GetVariantVerString (variant);
			const QStringList& features = capsDb->Get (ver);
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
			iq.setExtensions (QXmppElementList () << x);
			client ()->sendPacket (iq);
		}
		else
		{
			QXmppMessage msg;
			msg.setTo (to->GetHumanReadableID ());
			msg.setBody (message);
			msg.setExtensions (QXmppElementList () << x);
			client ()->sendPacket (msg);
		}
	}
}
}
}
