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

#include "annotationsmanager.h"
#include "xmppannotationsmanager.h"
#include "clientconnection.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	AnnotationsManager::AnnotationsManager (ClientConnection *parent)
	: QObject (parent)
	, ClientConn_ (parent)
	, XMPPAnnManager_ (new XMPPAnnotationsManager)
	{
		ClientConn_->GetClient ()->addExtension (XMPPAnnManager_);

		connect (XMPPAnnManager_,
				SIGNAL (notesReceived (const QList<XMPPAnnotationsIq::NoteItem>&)),
				this,
				SLOT (handleNotesReceived (const QList<XMPPAnnotationsIq::NoteItem>&)));
	}

	XMPPAnnotationsIq::NoteItem AnnotationsManager::GetNote (const QString& jid) const
	{
		return JID2Note_ [jid];
	}

	void AnnotationsManager::SetNote (const QString& jid, const XMPPAnnotationsIq::NoteItem& note)
	{
		JID2Note_ [jid] = note;
		XMPPAnnManager_->SetNotes (JID2Note_.values ());
	}

	void AnnotationsManager::refetchNotes ()
	{
		JID2Note_.clear ();
		XMPPAnnManager_->RequestNotes ();
	}

	void AnnotationsManager::handleNotesReceived (const QList<XMPPAnnotationsIq::NoteItem>& notes)
	{
		Q_FOREACH (const auto& item, notes)
			JID2Note_ [item.GetJid ()] = item;
	}
}
}
}
