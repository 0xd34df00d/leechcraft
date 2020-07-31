/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "accstatusrestorer.h"
#include <QXmppClient.h>
#include "clientconnection.h"

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	AccStatusRestorer::AccStatusRestorer (const GlooxAccountState& state, ClientConnection_wptr client)
	: State_ (state)
	, Client_ (client)
	{
		auto sharedPtr = client.lock ();
		if (!sharedPtr || state.State_ == SOffline)
		{
			deleteLater ();
			return;
		}

		auto xmppClient = sharedPtr->GetClient ();
		if (xmppClient->isConnected ())
			connect (xmppClient,
					SIGNAL (disconnected ()),
					this,
					SLOT (handleDisconnected ()));
		else
			handleDisconnected ();
	}

	void AccStatusRestorer::handleDisconnected ()
	{
		auto sharedPtr = Client_.lock ();
		if (!sharedPtr)
		{
			deleteLater ();
			return;
		}

		sharedPtr->SetState (State_);
	}
}
}
}
