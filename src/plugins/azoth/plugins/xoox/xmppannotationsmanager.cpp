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

#include "xmppannotationsmanager.h"
#include <QDomElement>
#include <QXmppClient.h>
#include <QXmppConstants.h>

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	void XMPPAnnotationsManager::SetNotes (const QList<XMPPAnnotationsIq::NoteItem>& notes)
	{
		XMPPAnnotationsIq iq;
		iq.setType (QXmppIq::Set);
		iq.SetItems (notes);
		client ()->sendPacket (iq);
	}

	void XMPPAnnotationsManager::RequestNotes ()
	{
		XMPPAnnotationsIq iq;
		iq.setType (QXmppIq::Get);
		client ()->sendPacket (iq);
	}

	bool XMPPAnnotationsManager::handleStanza(const QDomElement& element)
	{
		if (element.tagName () != "iq")
			return false;

		const auto& query = element.firstChildElement ("query");
		if (query.firstChildElement ("storage").namespaceURI () != ns_rosternotes)
			return false;

		XMPPAnnotationsIq iq;
		iq.parse (element);
		emit notesReceived (iq.GetItems ());
		return true;
	}
}
}
}
