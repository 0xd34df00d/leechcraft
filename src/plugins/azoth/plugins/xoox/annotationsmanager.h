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

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_ANNOTATIONSMANAGER_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_ANNOTATIONSMANAGER_H
#include <QObject>
#include <QHash>
#include <QXmppAnnotationsIq.h>

class QXmppAnnotationsManager;

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	class ClientConnection;

	class AnnotationsManager : public QObject
	{
		Q_OBJECT
		
		ClientConnection *ClientConn_;
		QXmppAnnotationsManager *XMPPAnnManager_;
		
		QHash<QString, QXmppAnnotationsIq::NoteItem> JID2Note_;
	public:
		AnnotationsManager (ClientConnection*);
		
		QXmppAnnotationsIq::NoteItem GetNote (const QString&) const;
		void SetNote (const QString&, const QXmppAnnotationsIq::NoteItem&);
	public slots:
		void refetchNotes ();
	private slots:
		void handleNotesReceived (const QList<QXmppAnnotationsIq::NoteItem>&);
	};
}
}
}

#endif
