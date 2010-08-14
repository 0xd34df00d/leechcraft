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

#include "datastorageserver.h"
#include "serverchainhandler.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Syncer
		{
			DataStorageServer::DataStorageServer (QObject *parent)
			: DataStorageBase (parent)
			{
			}

			void DataStorageServer::sync (const QByteArray& chain)
			{
				// TODO correctly throw a correct exception
				if (ChainHandlers_.contains (chain))
					return;

				ServerChainHandler *handler = new ServerChainHandler (chain, this);
				connect (handler,
						SIGNAL (gotNewDeltas (const Sync::Deltas_t&, const QByteArray&)),
						this,
						SIGNAL (gotNewDeltas (const Sync::Deltas_t&, const QByteArray&)));
				connect (handler,
						SIGNAL (deltasRequired (Sync::Deltas_t*, const QByteArray&)),
						this,
						SIGNAL (deltasRequired (Sync::Deltas_t*, const QByteArray&)));
				ChainHandlers_ [chain] = handler;
				handler->Sync ();
			}
		}
	}
}
