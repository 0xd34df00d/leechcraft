/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "pingmanager.h"
#include <QElapsedTimer>
#include <QDomElement>
#include <QXmppClient.h>
#include <QXmppPingIq.h>

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	bool PingManager::handleStanza (const QDomElement& elem)
	{
		const auto& id = elem.attribute ("id");
		if (!Stanza2Info_.contains (id) ||
				elem.attribute ("type") != "result")
			return false;

		const auto& info = Stanza2Info_.take (id);
		info.Handler_ (info.Timer_->elapsed ());

		return false;
	}

	void PingManager::Ping (const QString& jid, const ReplyHandler_f& cb)
	{
		QXmppPingIq iq;
		iq.setTo (jid);
		client ()->sendPacket (iq);

		const auto& timer = std::make_shared<QElapsedTimer> ();
		timer->start ();
		Stanza2Info_ [iq.id ()] = PingInfo { timer, cb };
	}
}
}
}
