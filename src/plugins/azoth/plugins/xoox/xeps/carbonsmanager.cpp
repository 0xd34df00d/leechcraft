/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "carbonsmanager.h"
#include <QDomElement>
#include <QXmppClient.h>
#include <QXmppMessage.h>
#include <QXmppUtils.h>
#include <util/sll/qtutil.h>
#include "util.h"

namespace LC::Azoth::Xoox
{
	namespace
	{
		const QString NsCarbons = "urn:xmpp:carbons:2"_qs;
	}

	QStringList CarbonsManager::discoveryFeatures () const
	{
		return { NsCarbons };
	}

	bool CarbonsManager::handleStanza (const QDomElement& stanza)
	{
		if (stanza.tagName () == "iq"_qs &&
				stanza.attribute ("id"_qs) == LastReqId_)
		{
			LastReqId_.clear ();

			if (stanza.attribute ("type"_qs) == "result"_qs)
			{
				LastConfirmedState_ = LastReqState_;
				emit stateChanged (LastConfirmedState_);
			}
			else
			{
				QXmppIq iq;
				iq.parse (stanza);
				emit stateChangeError (iq);
			}

			return true;
		}

		return false;
	}

	void CarbonsManager::SetEnabled (bool enabled)
	{
		QXmppIq iq { QXmppIq::Set };

		QXmppElement elem;
		elem.setTagName (enabled ? "enable"_qs : "disable"_qs);
		elem.setAttribute ("xmlns"_qs, NsCarbons);
		iq.setExtensions ({ elem });

		client ()->sendPacket (iq);

		LastReqId_ = iq.id ();
		LastReqState_ = enabled;
	}

	bool CarbonsManager::IsEnabled () const
	{
		return LastConfirmedState_;
	}

	bool CarbonsManager::CheckMessage (const QXmppMessage& msg)
	{
		if (const auto& fromBare = QXmppUtils::jidToBareJid (msg.from ());
			!fromBare.isEmpty () && fromBare != client ()->configuration ().jidBare ())
			return false;

		for (const auto& extension : msg.extensions ())
		{
			if (extension.attribute ("xmlns"_qs) != NsCarbons)
				continue;

			const auto& tag = extension.tagName ();
			if (tag == "received"_qs || tag == "sent"_qs)
			{
				HandleMessage (extension);
				return true;
			}
		}

		return false;
	}

	void CarbonsManager::HandleMessage (const QXmppElement& extElem)
	{
		const auto& msg = XooxUtil::Forwarded2Message (extElem);
		if (msg.to ().isEmpty ())
			return;

		emit gotMessage (msg);
	}
}
