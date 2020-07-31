/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "annotationsmanager.h"
#include "xeps/xmppannotationsmanager.h"
#include "clientconnection.h"
#include "clientconnectionextensionsmanager.h"

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	AnnotationsManager::AnnotationsManager (ClientConnection& conn, QObject *parent)
	: QObject { parent }
	, XMPPAnnManager_ { conn.Exts ().Get<XMPPAnnotationsManager> () }
	{
		connect (&XMPPAnnManager_,
				&XMPPAnnotationsManager::notesReceived,
				this,
				[this] (const QList<XMPPAnnotationsIq::NoteItem>& notes)
				{
					for (const auto& item : notes)
						JID2Note_ [item.GetJid ()] = item;
				});

		connect (&conn,
				&ClientConnection::connected,
				this,
				[this]
				{
					JID2Note_.clear ();
					XMPPAnnManager_.RequestNotes ();
				});
	}

	XMPPAnnotationsIq::NoteItem AnnotationsManager::GetNote (const QString& jid) const
	{
		return JID2Note_ [jid];
	}

	void AnnotationsManager::SetNote (const QString& jid, const XMPPAnnotationsIq::NoteItem& note)
	{
		JID2Note_ [jid] = note;
		XMPPAnnManager_.SetNotes (JID2Note_.values ());
	}
}
}
}
