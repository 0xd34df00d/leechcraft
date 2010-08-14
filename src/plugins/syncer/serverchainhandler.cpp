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
#include <QtDebug>
#include <interfaces/isyncable.h>
#include <plugininterface/syncops.h>
#include "serverconnection.h"
#include "core.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Syncer
		{
			ServerChainHandler::ServerChainHandler (const QByteArray& chain, QObject *parent)
			: QObject (parent)
			, ServerConnection_ (new ServerConnection (chain, this))
			, Chain_ (chain)
			{
				Idle_ = new QState ();
				ConnectionError_ = new QState ();
				LoginPending_ = new QState ();
				LoginError_ = new QState ();
				Running_ = new QState ();
				ReqMaxDeltaPending_ = new QState ();
				GetDeltasPending_ = new QState ();
				ProcessDeltas_ = new QState ();
				PutDeltasPending_ = new QState ();

				SM_.addState (Idle_);
				SM_.addState (ConnectionError_);
				SM_.addState (LoginPending_);
				SM_.addState (LoginError_);
				SM_.addState (Running_);
				SM_.addState (ReqMaxDeltaPending_);
				SM_.addState (GetDeltasPending_);
				SM_.addState (ProcessDeltas_);
				SM_.addState (PutDeltasPending_);
				SM_.setInitialState (Idle_);

				ConnectionError_->addTransition (Idle_);
				Idle_->addTransition (this,
						SIGNAL (initiated ()), LoginPending_);
				connect (LoginPending_,
						SIGNAL (entered ()),
						ServerConnection_,
						SLOT (performLogin ()));
				LoginPending_->addTransition (this,
						SIGNAL (fail ()), LoginError_);
				LoginError_->addTransition (Idle_);
				LoginPending_->addTransition (this,
						SIGNAL (success ()), Running_);
				Running_->addTransition (ReqMaxDeltaPending_);
				connect (ReqMaxDeltaPending_,
						SIGNAL (entered ()),
						ServerConnection_,
						SLOT (reqMaxDelta ()));
				ReqMaxDeltaPending_->addTransition (this,
						SIGNAL (fail ()), ConnectionError_);
				ReqMaxDeltaPending_->addTransition (this,
						SIGNAL (hasNewDeltas ()), GetDeltasPending_);
				ReqMaxDeltaPending_->addTransition (this,
						SIGNAL (noNewDeltas ()), PutDeltasPending_);
				connect (GetDeltasPending_,
						SIGNAL (entered ()),
						this,
						SLOT (getNewDeltas ()));
				GetDeltasPending_->addTransition (this,
						SIGNAL (deltasReceived ()), ProcessDeltas_);
				ProcessDeltas_->addTransition (this,
						SIGNAL (deltasProcessed ()), PutDeltasPending_);
				connect (PutDeltasPending_,
						SIGNAL (entered ()),
						this,
						SLOT (handlePutDeltas ()));

				SM_.start ();

				connect (ServerConnection_,
						SIGNAL (deltasReceived (const QList<QByteArray>&)),
						this,
						SLOT (handleDeltasReceived (const QList<QByteArray>&)),
						Qt::QueuedConnection);
				connect (ServerConnection_,
						SIGNAL (maxDeltaIDReceived (quint32)),
						this,
						SLOT (handleMaxDeltaIDReceived (quint32)));
				connect (ServerConnection_,
						SIGNAL (success (const QList<QByteArray>&)),
						this,
						SLOT (handleSuccess (const QList<QByteArray>&)));
				connect (ServerConnection_,
						SIGNAL (fail ()),
						this,
						SIGNAL (fail ()));
			}

			void ServerChainHandler::Sync ()
			{
				emit initiated ();
			}

			void ServerChainHandler::getNewDeltas ()
			{
				quint32 lastId = Core::Instance ().GetLastID (Chain_);
				ServerConnection_->getDeltas (lastId);
			}

			void ServerChainHandler::handleSuccess (const QList<QByteArray>& data)
			{
				QSet<QAbstractState*> conf = SM_.configuration ();
				if (conf.contains (ReqMaxDeltaPending_))
				{
					quint32 deltaId = 0;
					if (!data.size ())
					{
						qWarning () << Q_FUNC_INFO
								<< "insufficient number of lists for ReqMaxDeltaPending_ state";
						emit fail ();
						return;
					}
					QDataStream ds (data.at (0));
					ds >> deltaId;
					handleMaxDeltaIDReceived (deltaId);
				}
				else if (conf.contains (GetDeltasPending_))
				{
					emit deltasReceived ();
					handleDeltasReceived (data);
				}
			}

			void ServerChainHandler::handleMaxDeltaIDReceived (quint32 deltaId)
			{
				quint32 lastId = Core::Instance ().GetLastID (Chain_);
				if (lastId < deltaId)
					emit hasNewDeltas ();
				else if (lastId == deltaId)
					emit noNewDeltas ();
				else
				{
					qWarning () << Q_FUNC_INFO
							<< "our last ID is greater then remote last delta ID:"
							<< lastId
							<< deltaId;
					emit fail ();
				}
			}

			void ServerChainHandler::handleDeltasReceived (const QList<QByteArray>& deltas)
			{
				Sync::Deltas_t parsed;
				Q_FOREACH (const QByteArray& ba, deltas)
				{
					Sync::Delta delta;
					QDataStream ds (ba);
					try
					{
						ds >> delta;
					}
					catch (const std::exception& e)
					{
						qWarning () << Q_FUNC_INFO
								<< e.what ();
						break;
					}
					parsed << delta;
				}

				emit gotNewDeltas (parsed, Chain_);
				emit deltasProcessed ();
			}

			void ServerChainHandler::handlePutDeltas ()
			{
				Sync::Deltas_t deltas;
				emit deltasRequired (&deltas, Chain_);

				QList<QByteArray> dBytes;
				//ServerConnection_->putDeltas (dBytes);
			}
		}
	}
}
