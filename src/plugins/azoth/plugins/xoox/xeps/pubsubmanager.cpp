/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "pubsubmanager.h"
#include <algorithm>
#include <QDomElement>
#include <QtDebug>
#include <QXmppClient.h>

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	const QString NsPubSub = "http://jabber.org/protocol/pubsub";
	const QString NsPubSubEvent = "http://jabber.org/protocol/pubsub#event";

	void PubSubManager::RegisterCreator (const QString& node, const Creator_t& creator)
	{
		Node2Creator_ [node] = creator;
		SetAutosubscribe (node, false);
	}

	void PubSubManager::SetAutosubscribe (QString node, bool enabled)
	{
		node += "+notify";

		if (enabled)
			AutosubscribeNodes_ << node;
		else
			AutosubscribeNodes_.remove (node);
	}

	void PubSubManager::PublishEvent (PEPEventBase *event)
	{
		QXmppElement publish;
		publish.setTagName ("publish");
		publish.setAttribute ("node", event->Node ());
		publish.appendChild (event->ToXML ());

		QXmppElement pubsub;
		pubsub.setTagName ("pubsub");
		pubsub.setAttribute ("xmlns", NsPubSub);
		pubsub.appendChild (publish);

		QXmppIq iq (QXmppIq::Set);
		iq.setExtensions (QXmppElementList () << pubsub);
		client ()->sendPacket (iq);
	}

	void PubSubManager::RequestItem (const QString& jid,
			const QString& node, const QString& id)
	{
		QXmppElement item;
		item.setTagName ("item");
		item.setAttribute ("id", id);

		QXmppElement items;
		items.setTagName ("items");
		items.setAttribute ("node", node);
		items.appendChild (item);

		QXmppElement pubsub;
		pubsub.setTagName ("pubsub");
		pubsub.setAttribute ("xmlns", NsPubSub);
		pubsub.appendChild (items);

		QXmppIq iq (QXmppIq::Get);
		iq.setTo (jid);
		iq.setExtensions (QXmppElementList () << pubsub);
		client ()->sendPacket (iq);
	}

	QStringList PubSubManager::discoveryFeatures () const
	{
		QStringList result { NsPubSub };
		std::copy (Node2Creator_.keyBegin (), Node2Creator_.keyEnd (), std::back_inserter (result));
		std::copy (AutosubscribeNodes_.begin (), AutosubscribeNodes_.end (), std::back_inserter (result));
		return result;
	}

	bool PubSubManager::handleStanza (const QDomElement& elem)
	{
		if (elem.tagName () == "message")
			return HandleMessage (elem);
		else if (elem.tagName () == "iq")
			return HandleIq (elem);
		else
			return false;
	}

	bool PubSubManager::HandleIq (const QDomElement& elem)
	{
		const QDomElement& pubsub = elem.firstChildElement ("pubsub");
		if (pubsub.namespaceURI () != NsPubSub)
			return false;

		ParseItems (pubsub.firstChildElement ("items"), elem.attribute ("from"));

		return true;
	}

	bool PubSubManager::HandleMessage (const QDomElement& elem)
	{
		if (elem.tagName () != "message" || elem.attribute ("type") != "headline")
			return false;

		const QDomElement& event = elem.firstChildElement ("event");
		if (event.namespaceURI () != NsPubSubEvent)
			return false;

		ParseItems (event.firstChildElement ("items"), elem.attribute ("from"));

		return true;
	}

	void PubSubManager::ParseItems (QDomElement items, const QString& from)
	{
		while (!items.isNull ())
		{
			const QString& node = items.attribute ("node");
			if (!Node2Creator_.contains (node))
			{
				items = items.nextSiblingElement ("items");
				continue;
			}

			QDomElement item = items.firstChildElement ("item");
			while (!item.isNull ())
			{
				PEPEventBase *eventObj = Node2Creator_ [node] ();
				eventObj->Parse (item);

				emit gotEvent (from, eventObj);

				delete eventObj;

				item = item.nextSiblingElement ("item");
			}

			items = items.nextSiblingElement ("items");
		}
	}
}
}
}
