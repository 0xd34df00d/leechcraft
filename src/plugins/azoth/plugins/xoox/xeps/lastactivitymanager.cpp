/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "lastactivitymanager.h"
#include <QDomElement>
#include <QXmppClient.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/azoth/ilastactivityprovider.h>
#include "core.h"

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	const QString NsLastActivity = "jabber:iq:last";

	QStringList LastActivityManager::discoveryFeatures () const
	{
		return { NsLastActivity };
	}

	bool LastActivityManager::handleStanza (const QDomElement& elem)
	{
		if (elem.tagName () != "iq")
				return false;

		const auto& query = elem.firstChildElement ("query");
		if (query.namespaceURI () != NsLastActivity)
			return false;

		const auto& from = elem.attribute ("from");

		if (elem.attribute ("type") == "get")
		{
			const auto pMgr = Core::Instance ().GetProxy ()->GetPluginsManager ();
			const auto prov = pMgr->GetAllCastableTo<ILastActivityProvider*> ().value (0);

			if (!prov)
				return false;

			auto iq = CreateIq (from, std::max (prov->GetInactiveSeconds (), 0));
			iq.setType (QXmppIq::Result);
			iq.setId (elem.attribute ("id"));

			client ()->sendPacket (iq);
		}
		else if (elem.attribute ("type") == "result" &&
				query.hasAttribute ("seconds"))
			emit gotLastActivity (from, query.attribute ("seconds").toInt ());

		return true;
	}

	QString LastActivityManager::RequestLastActivity (const QString& jid)
	{
		auto iq = CreateIq (jid);
		iq.setType (QXmppIq::Get);
		client ()->sendPacket (iq);

		return iq.id ();
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
		iq.setExtensions ({ queryElem });

		return iq;
	}
}
}
}
