/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "legacyentitytimeext.h"
#include <QDomElement>
#include <QDateTime>
#include <QXmppClient.h>

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	const QString NsLegacyEntityTime = "jabber:iq:time";

	QStringList LegacyEntityTimeExt::discoveryFeatures () const
	{
		return QStringList (NsLegacyEntityTime);
	}

	bool LegacyEntityTimeExt::handleStanza (const QDomElement& elem)
	{
		if (elem.tagName () != "iq" ||
				elem.attribute ("type") != "get")
			return false;

		if (elem.firstChildElement ("query").namespaceURI () != NsLegacyEntityTime)
			return false;

		const QString& from = elem.attribute ("from");
		if (from.isEmpty ())
			return false;

		const QDateTime& date = QDateTime::currentDateTime ().toUTC ();

		QXmppElement utcElem;
		utcElem.setTagName ("utc");
		utcElem.setValue (date.toString ("yyyyMMddThh:mm:ss"));

		const auto& displayStr = QDateTime::currentDateTime ().toString ();
		QXmppElement displayElem;
		displayElem.setTagName ("display");
		displayElem.setValue (displayStr);

		QXmppElement queryElem;
		queryElem.setTagName ("query");
		queryElem.setAttribute ("xmlns", NsLegacyEntityTime);
		queryElem.appendChild (utcElem);
		queryElem.appendChild (displayElem);

		QXmppIq iq (QXmppIq::Result);
		iq.setTo (from);
		iq.setId (elem.attribute ("id"));
		iq.setExtensions (QXmppElementList () << queryElem);

		client ()->sendPacket (iq);

		return true;
	}
}
}
}
