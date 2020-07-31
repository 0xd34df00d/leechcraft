/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "xmppbobmanager.h"
#include <QDomElement>
#include <QXmppClient.h>
#include "xmppbobiq.h"

namespace LC
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
			else
				return false;
		}
		else if ((tagName == "message" || tagName == "presence") &&
				XMPPBobIq::IsBobIq (stanza))
			bobIq.parse (stanza);
		else
			return false;

		if (bobIq.GetData ().isNull ())
		{
			qWarning () << Q_FUNC_INFO
					<< "got an empty BoB"
					<< tagName
					<< stanza.attribute ("from");
			return false;
		}

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

		qWarning () << Q_FUNC_INFO
				<< "unable to find"
				<< key
				<< "among"
				<< BobCache_.keys ();
		return QByteArray ();
	}
}
}
}
