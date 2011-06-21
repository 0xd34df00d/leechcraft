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

#include "pubsubmanager.h"
#include <QDomElement>
#include <QtDebug>
#include <QXmppClient.h>

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	const QString NsPubSub = "http://jabber.org/protocol/pubsub";
	const QString NsPubSubEvent = "http://jabber.org/protocol/pubsub#event";

	void PubSubManager::RegisterCreator (const QString& node,
			boost::function<PEPEventBase* ()> creator)
	{
		Node2Creator_ [node] = creator;
		SetAutosubscribe (node, false);
	}
	
	void PubSubManager::SetAutosubscribe (const QString& node, bool enabled)
	{
		AutosubscribeNodes_ [node] = enabled;
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
		iq.setExtensions (pubsub);
		client ()->sendPacket (iq);
	}

	QStringList PubSubManager::discoveryFeatures () const
	{
		QStringList result;
		Q_FOREACH (const QString& node, Node2Creator_.keys ())
		{
			result << node;
			if (AutosubscribeNodes_ [node])
				result << node + "+notify";
		}
		return result;
	}
	
	bool PubSubManager::handleStanza (const QDomElement& elem)
	{
		if (elem.tagName () != "message" || elem.attribute ("type") != "headline")
			return false;
		
		const QDomElement& event = elem.firstChildElement ("event");
		if (event.namespaceURI () != NsPubSubEvent)
			return false;
		
		const QString& from = elem.attribute ("from");
		
		QDomElement items = event.firstChildElement ("items");
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
		
		return true;
	}
}
}
}
