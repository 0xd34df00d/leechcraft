/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "discomanagerwrapper.h"
#include <QXmppDiscoveryManager.h>
#include "clientconnection.h"
#include "clientconnectionerrormgr.h"

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	DiscoManagerWrapper::DiscoManagerWrapper (QXmppDiscoveryManager *mgr, ClientConnection *conn)
	: QObject { conn }
	, Mgr_ { mgr }
	, Conn_ { conn }
	{
		connect (Mgr_,
				SIGNAL (infoReceived (const QXmppDiscoveryIq&)),
				this,
				SLOT (handleDiscoInfo (const QXmppDiscoveryIq&)));
		connect (Mgr_,
				SIGNAL (itemsReceived (const QXmppDiscoveryIq&)),
				this,
				SLOT (handleDiscoItems (const QXmppDiscoveryIq&)));
	}

	void DiscoManagerWrapper::RequestInfo (const QString& jid,
			DiscoCallback_t callback, bool report, const QString& node)
	{
		AwaitingDiscoInfo_ [jid] = callback;

		const auto& id = Mgr_->requestInfo (jid, node);
		if (report)
			Conn_->GetErrorManager ()->Whitelist (id);
	}

	void DiscoManagerWrapper::RequestItems (const QString& jid,
			DiscoCallback_t callback, bool report, const QString& node)
	{
		AwaitingDiscoItems_ [jid] = callback;

		const auto& id = Mgr_->requestItems (jid, node);
		if (report)
			Conn_->GetErrorManager ()->Whitelist (id);
	}

	void DiscoManagerWrapper::handleDiscoInfo (const QXmppDiscoveryIq& iq)
	{
		const auto& jid = iq.from ();
		if (AwaitingDiscoInfo_.contains (jid))
			AwaitingDiscoInfo_.take (jid) (iq);
	}

	void DiscoManagerWrapper::handleDiscoItems (const QXmppDiscoveryIq& iq)
	{
		const auto& jid = iq.from ();
		if (AwaitingDiscoItems_.contains (jid))
			AwaitingDiscoItems_.take (jid) (iq);
	}
}
}
}
