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
#include <QFinalState>
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
			, NumLastSentOut_ (0)
			{
				Idle_ = new QState ();
				ConnectionError_ = new QFinalState ();
				LoginPending_ = new QState ();
				LoginError_ = new QFinalState ();
				Running_ = new QState ();
				ReqMaxDeltaPending_ = new QState ();
				GetDeltasPending_ = new QState ();
				ProcessDeltas_ = new QState ();
				PutDeltasPending_ = new QState ();
				Finish_ = new QFinalState ();

				SM_.addState (Idle_);
				SM_.addState (ConnectionError_);
				SM_.addState (LoginPending_);
				SM_.addState (LoginError_);
				SM_.addState (Running_);
				SM_.addState (ReqMaxDeltaPending_);
				SM_.addState (GetDeltasPending_);
				SM_.addState (ProcessDeltas_);
				SM_.addState (PutDeltasPending_);
				SM_.addState (Finish_);
				SM_.setInitialState (Idle_);

				Idle_->addTransition (this,
						SIGNAL (initiated ()), LoginPending_);
				connect (LoginPending_,
						SIGNAL (entered ()),
						ServerConnection_,
						SLOT (performLogin ()));
				LoginPending_->addTransition (this,
						SIGNAL (fail ()), LoginError_);
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
				PutDeltasPending_->addTransition (ServerConnection_,
						SIGNAL (deltaOutOfOrder ()), Running_);
				PutDeltasPending_->addTransition (this,
						SIGNAL (fail ()), ConnectionError_);
				PutDeltasPending_->addTransition (this,
						SIGNAL (success ()), Finish_);

				connect (&SM_,
						SIGNAL (finished ()),
						this,
						SLOT (handleFinished ()));

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
				else if (conf.contains (PutDeltasPending_))
				{
					if (NumLastSentOut_)
					{
						emit successfullySentDeltas (NumLastSentOut_, Chain_);
						NumLastSentOut_ = 0;
					}
					emit success ();
				}
				else
					emit success ();
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

				NumLastSentOut_ = deltas.size ();

				if (!deltas.size ())
				{
					emit success ();
					return;
				}

				quint32 firstId = deltas.at (0).ID_;

				QList<QByteArray> dBytes;
				Q_FOREACH (const Sync::Delta& delta, deltas)
				{
					QByteArray ba;
					{
						QDataStream ds (&ba, QIODevice::WriteOnly);
						ds << delta;
					}
					dBytes << ba;
				}
				ServerConnection_->putDeltas (dBytes, firstId);
			}

			void ServerChainHandler::handleFinished ()
			{
				QSet<QAbstractState*> conf = SM_.configuration ();
				if (conf.contains (LoginError_))
					emit loginError ();
				else if (conf.contains (ConnectionError_))
					emit connectionError ();
				else if (conf.contains (Finish_))
					emit finishedSuccessfully ();
			}
		}
	}
}
