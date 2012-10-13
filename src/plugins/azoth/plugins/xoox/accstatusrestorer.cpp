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

#include "accstatusrestorer.h"
#include <QXmppClient.h>
#include "clientconnection.h"

namespace LeechCraft
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
