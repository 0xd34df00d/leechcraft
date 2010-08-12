/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2010  Georg Rudoy
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

#include "serverchainhandler.h"
#include "serverconnection.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Syncer
		{
			ServerChainHandler::ServerChainHandler (const QString& chain, QObject *parent)
			: QObject (parent)
			, ServerConnection_ (new ServerConnection (chain, this))
			{
				QState *idle = new QState ();
				QState *connectionError = new QState ();
				QState *loginPending = new QState ();
				QState *loginError = new QState ();
				QState *running = new QState ();
				QState *reqMaxDeltaPending = new QState ();

				SM_.addState (idle);
				SM_.addState (connectionError);
				SM_.addState (loginPending);
				SM_.addState (loginError);
				SM_.addState (running);
				SM_.addState (reqMaxDeltaPending);
				SM_.setInitialState (idle);

				connectionError->addTransition (idle);
				idle->addTransition (this,
						SIGNAL (initiated ()), loginPending);
				connect (loginPending,
						SIGNAL (entered ()),
						ServerConnection_,
						SLOT (performLogin ()));
				loginPending->addTransition (ServerConnection_,
						SIGNAL (fail ()), loginError);
				loginError->addTransition (idle);
				loginPending->addTransition (ServerConnection_,
						SIGNAL (success ()), running);
				running->addTransition (reqMaxDeltaPending);
				connect (reqMaxDeltaPending,
						SIGNAL (entered ()),
						ServerConnection_,
						SLOT (reqMaxDelta ()));
				reqMaxDeltaPending->addTransition (ServerConnection_,
						SIGNAL (fail ()), connectionError);

				SM_.start ();
			}

			void ServerChainHandler::Sync ()
			{
				emit initiated ();
			}
		}
	}
}
