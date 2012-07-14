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

#include "legacyentitytimeext.h"
#include <QDomElement>
#include <QDateTime>
#include <QXmppClient.h>

namespace LeechCraft
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
		
		const QString& displayStr = "Your client/bot sucks since it "
				"uses the long-deprecated XEP-0090. Upgrade your code. "
				"Ah, and, regarding your question, it's " +
				QDateTime::currentDateTime ().toString () + " here";
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
		iq.setExtensions (queryElem);
		
		client ()->sendPacket (iq);
		
		return true;
	}
}
}
}