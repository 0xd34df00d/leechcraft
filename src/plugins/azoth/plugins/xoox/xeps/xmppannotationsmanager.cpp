/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "xmppannotationsmanager.h"
#include <QDomElement>
#include <QXmppClient.h>

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	const QString NSRosterNotes = "storage:rosternotes";

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
		if (query.firstChildElement ("storage").namespaceURI () != NSRosterNotes)
			return false;

		XMPPAnnotationsIq iq;
		iq.parse (element);
		emit notesReceived (iq.GetItems ());
		return true;
	}
}
}
}
