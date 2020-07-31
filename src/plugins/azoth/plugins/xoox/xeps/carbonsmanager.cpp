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
#include "util.h"

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	namespace
	{
		const QString NsCarbons { "urn:xmpp:carbons:2" };
	}

	QStringList CarbonsManager::discoveryFeatures () const
	{
		return { NsCarbons };
	}

	bool CarbonsManager::handleStanza (const QDomElement& stanza)
	{
		if (stanza.tagName () == "iq" &&
				stanza.attribute ("id") == LastReqId_)
		{
			LastReqId_.clear ();

			if (stanza.attribute ("type") == "result")
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
		elem.setTagName (enabled ? "enable" : "disable");
		elem.setAttribute ("xmlns", NsCarbons);
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
		for (const auto& extension : msg.extensions ())
		{
			const auto& tag = extension.tagName ();
			if ((tag == "received" || tag == "sent") &&
				extension.attribute ("xmlns") == NsCarbons)
			{
				HandleMessage (extension);
				return true;
			}
		}

		return false;
	}

	void CarbonsManager::ExcludeMessage (QXmppMessage& msg)
	{
		QXmppElement privElem;
		privElem.setTagName ("private");
		privElem.setAttribute ("xmlns", NsCarbons);

		auto extensions = msg.extensions ();
		extensions.append (privElem);
		msg.setExtensions (extensions);
	}

	void CarbonsManager::HandleMessage (const QXmppElement& extElem)
	{
		const auto& msg = XooxUtil::Forwarded2Message (extElem);
		if (msg.to ().isEmpty ())
			return;

		emit gotMessage (msg);
	}
}
}
}
