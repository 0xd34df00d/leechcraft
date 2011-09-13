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

#include "jabbersearchmanager.h"
#include <QDomElement>
#include <QXmppClient.h>
#include "util.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	const QString NsJabberSearch = "jabber:iq:search";

	JabberSearchManager::Item::Item ()
	{
	}

	JabberSearchManager::Item::Item (const QString& jid,
			const QString& first, const QString& last,
			const QString& nick, const QString& email)
	{
		Dictionary_ ["JID"] = jid;
		Dictionary_ [tr ("First name")] = first;
		Dictionary_ [tr ("Last name")] = last;
		Dictionary_ [tr ("Nick")] = nick;
		Dictionary_ [tr ("E-Mail")] = email;
	}

	bool JabberSearchManager::handleStanza (const QDomElement& elem)
	{
		if (elem.tagName () != "iq" ||
				elem.attribute ("type") != "result")
			return false;

		const QString& id = elem.attribute ("id");
		if (FieldRequests_.remove (id))
		{
			const QDomElement& formElem = elem.firstChildElement ("query");
			if (formElem.attribute ("xmlns") != NsJabberSearch)
				return false;

			emit gotSearchFields (elem.attribute ("from"), QXmppElement (formElem));

			return true;
		}
		else if (SearchRequests_.remove (id))
		{
			const QDomElement& items = elem.firstChildElement ("query");
			if (items.attribute ("xmlns") != NsJabberSearch)
				return false;

			QList<Item> result;

			QDomElement item = items.firstChildElement ("item");
			while (!item.isNull ())
			{
				result << Item (item.attribute ("jid"),
						item.firstChildElement ("first").text (),
						item.firstChildElement ("last").text (),
						item.firstChildElement ("nick").text (),
						item.firstChildElement ("email").text ());

				item = item.nextSiblingElement ("item");
			}

			emit gotItems (elem.attribute ("from"), result);

			return true;
		}
		else
			return false;
	}

	void JabberSearchManager::RequestSearchFields (const QString& server)
	{
		QXmppIq iq (QXmppIq::Get);
		iq.setTo (server);

		QXmppElement query;
		query.setTagName ("query");
		query.setAttribute ("xmlns", NsJabberSearch);

		iq.setExtensions (query);

		FieldRequests_ << iq.id ();

		client ()->sendPacket (iq);
	}

	void JabberSearchManager::SubmitSearchRequest (const QString& server, QXmppElement elem)
	{
		QXmppIq iq (QXmppIq::Set);
		iq.setTo (server);
		iq.setExtensions (elem);

		SearchRequests_ << iq.id ();

		client ()->sendPacket (iq);
	}
}
}
}
