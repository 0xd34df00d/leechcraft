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

#include "lastactivitymanager.h"
#include <QDomElement>
#include <QXmppClient.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/azoth/ilastactivityprovider.h>
#include "core.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	const QString NsLastActivity = "jabber:iq:last";

	QStringList LastActivityManager::discoveryFeatures () const
	{
		return QStringList (NsLastActivity);
	}

	bool LastActivityManager::handleStanza (const QDomElement& elem)
	{
		if (elem.tagName () != "iq")
				return false;

		const QDomElement& query = elem.firstChildElement ("query");
		if (query.namespaceURI () != NsLastActivity)
			return false;

		const QString& from = elem.attribute ("from");

		if (elem.attribute ("type") == "get")
		{
			IPluginsManager *pMgr = Core::Instance ()
					.GetProxy ()->GetPluginsManager ();
			ILastActivityProvider *prov = pMgr->
					GetAllCastableTo<ILastActivityProvider*> ().value (0);

			if (!prov)
				return false;

			QXmppIq iq = CreateIq (from, std::max (prov->GetInactiveSeconds (), 0));
			iq.setType (QXmppIq::Result);
			iq.setId (elem.attribute ("id"));

			client ()->sendPacket (iq);
		}
		else if (elem.attribute ("type") == "result" &&
				query.hasAttribute ("seconds"))
			emit gotLastActivity (from, query.attribute ("seconds").toInt ());

		return true;
	}

	void LastActivityManager::RequestLastActivity (const QString& jid)
	{
		QXmppIq iq = CreateIq (jid);
		iq.setType (QXmppIq::Get);
		client ()->sendPacket (iq);
	}

	QXmppIq LastActivityManager::CreateIq (const QString& to, int secs)
	{
		QXmppIq iq;
		iq.setTo (to);

		QXmppElement queryElem;
		queryElem.setTagName ("query");
		queryElem.setAttribute ("xmlns", NsLastActivity);
		if (secs != -1)
			queryElem.setAttribute ("seconds", QString::number (secs));
		iq.setExtensions (queryElem);

		return iq;
	}
}
}
}
