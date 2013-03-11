/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#include "xmppbobmanager.h"
#include <QDomElement>
#include <QXmppClient.h>
#include "xmppbobiq.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	const QString NSBob = "urn:xmpp:bob";

	XMPPBobManager::XMPPBobManager (unsigned int cacheSizeKb)
	: BobCache_ (cacheSizeKb)
	{
	}

	bool XMPPBobManager::handleStanza (const QDomElement& stanza)
	{
		XMPPBobIq bobIq;
		bool requestReceived = false;

		const auto& tagName = stanza.tagName ();
		if (tagName == "iq")
		{
			if (XMPPBobIq::IsBobIq (stanza))
			{
				requestReceived = true;
				bobIq.parse (stanza);
			}
			else if (XMPPBobIq::IsBobIq (stanza.firstChildElement ()))
				bobIq.parse (stanza.firstChildElement ());
		}
		else if ((tagName == "message" || tagName == "presence") &&
				XMPPBobIq::IsBobIq (stanza))
			bobIq.parse (stanza);
		else
			return false;

		BobCache_.insert (qMakePair (bobIq.GetCid (), bobIq.from ()),
				new QByteArray (bobIq.GetData ()),
				bobIq.GetData ().size () / 1024);

		if (requestReceived)
			emit bobReceived (bobIq);

		return requestReceived;
	}

	QStringList XMPPBobManager::discoveryFeatures () const
	{
		return QStringList (NSBob);
	}

	QString XMPPBobManager::RequestBob (const QString& jid, const QString& cid)
	{
		XMPPBobIq request;
		request.setType (QXmppIq::Get);
		request.setTo (jid);
		request.SetCid (cid);
		if (client ()->sendPacket (request))
			return request.id ();
		else
			return QString ();
	}

	QByteArray XMPPBobManager::Take (const QString& jid, const QString& cid)
	{
		QPair<QString, QString> key (cid, jid);
		if (BobCache_.contains (key))
			return *BobCache_ [key];
		return QByteArray ();
	}
}
}
}
