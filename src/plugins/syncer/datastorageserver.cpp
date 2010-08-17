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
#include <QtDebug>
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
				connect (handler,
						SIGNAL (successfullySentDeltas (quint32, const QByteArray&)),
						this,
						SIGNAL (successfullySentDeltas (quint32, const QByteArray&)));

				connect (handler,
						SIGNAL (loginError ()),
						this,
						SLOT (handleLoginError ()));
				connect (handler,
						SIGNAL (connectionError ()),
						this,
						SLOT (handleConnectionError ()));
				connect (handler,
						SIGNAL (finishedSuccessfully (quint32, quint32)),
						this,
						SLOT (handleFinishedSuccessfully (quint32, quint32)));

				ChainHandlers_ [chain] = handler;
				handler->Sync ();
			}

			void DataStorageServer::handleLoginError ()
			{
				QByteArray chain = GetChainForSender (sender ());
				if (chain.isEmpty ())
				{
					qWarning () << Q_FUNC_INFO
							<< "^^^^^^^^^^^^";
					return;
				}

				emit loginError (chain);
				ChainHandlers_.take (chain)->deleteLater ();
			}

			void DataStorageServer::handleConnectionError ()
			{
				QByteArray chain = GetChainForSender (sender ());
				if (chain.isEmpty ())
				{
					qWarning () << Q_FUNC_INFO
							<< "^^^^^^^^^^^^";
					return;
				}

				emit connectionError (chain);
				ChainHandlers_.take (chain)->deleteLater ();
			}

			void DataStorageServer::handleFinishedSuccessfully (quint32 sent, quint32 received)
			{
				QByteArray chain = GetChainForSender (sender ());
				if (chain.isEmpty ())
				{
					qWarning () << Q_FUNC_INFO
							<< "^^^^^^^^^^^^";
					return;
				}

				emit finishedSuccessfully (sent, received, chain);
				ChainHandlers_.take (chain)->deleteLater ();
			}

			QByteArray DataStorageServer::GetChainForSender (QObject *sender)
			{
				ServerChainHandler *handler = qobject_cast<ServerChainHandler*> (sender);
				if (!handler)
				{
					qWarning () << "sender is not a ServerChainHandler"
							<< sender;
					return QByteArray ();
				}

				QByteArray chain = ChainHandlers_.key (handler);
				if (chain.isEmpty ())
				{
					qWarning () << "no chain for handler"
							<< handler;
					handler->deleteLater ();
				}
				return chain;
			}
		}
	}
}
