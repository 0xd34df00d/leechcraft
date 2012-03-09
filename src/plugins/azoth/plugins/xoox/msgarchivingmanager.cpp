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

#include "msgarchivingmanager.h"
#include <QDomElement>
#include <QXmppClient.h>

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	const QString NsArchive = "urn:xmpp:archive";

	bool operator< (MsgArchMethod m1, MsgArchMethod m2)
	{
		return static_cast<int> (m1) < static_cast<int> (m2);
	}

	MsgArchPrefs::MsgArchPrefs ()
	: Valid_ (false)
	{
	}

	MsgArchivingManager::MsgArchivingManager (ClientConnection *conn)
	: Conn_ (conn)
	{
	}

	QStringList MsgArchivingManager::discoveryFeatures () const
	{
		return QStringList (NsArchive);
	}

	bool MsgArchivingManager::handleStanza (const QDomElement& elem)
	{
		return false;
	}

	void MsgArchivingManager::RequestPrefs ()
	{
		QXmppIq iq;

		QXmppElement elem;
		elem.setTagName ("pref");
		elem.setAttribute ("xmlns", NsArchive);
		iq.setExtensions (elem);

		client ()->sendPacket (iq);
	}
}
}
}
