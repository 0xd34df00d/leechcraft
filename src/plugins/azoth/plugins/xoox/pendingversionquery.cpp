/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "pendingversionquery.h"
#include <QXmppVersionManager.h>
#include <QXmppVersionIq.h>

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	PendingVersionQuery::PendingVersionQuery (QXmppVersionManager *vm, const QString& reqJid, QObject *parent)
	: QObject { parent }
	, Jid_ { reqJid }
	{
		connect (vm,
				SIGNAL (versionReceived (QXmppVersionIq)),
				this,
				SLOT (handleVersionReceived (QXmppVersionIq)));
	}

	void PendingVersionQuery::handleVersionReceived (const QXmppVersionIq& iq)
	{
		const auto& from = iq.from ();

		if (from == Jid_ ||
				(!Jid_.contains ('/') && from.startsWith (Jid_)))
		{
			emit versionReceived ();
			deleteLater ();
		}
	}
}
}
}
