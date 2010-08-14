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

#ifndef PLUGINS_SYNCER_SERVERCHAINHANDLER_H
#define PLUGINS_SYNCER_SERVERCHAINHANDLER_H
#include <QStateMachine>
#include <interfaces/isyncable.h>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Syncer
		{
			class ServerConnection;

			class ServerChainHandler : public QObject
			{
				Q_OBJECT

				QStateMachine SM_;
				ServerConnection *ServerConnection_;
				QByteArray Chain_;
				QState *Idle_;
				QState *ConnectionError_;
				QState *LoginPending_;
				QState *LoginError_;
				QState *Running_;
				QState *ReqMaxDeltaPending_;
				QState *GetDeltasPending_;
				QState *ProcessDeltas_;
				QState *PutDeltasPending_;
			public:
				ServerChainHandler (const QByteArray&, QObject*);
			public:
				void Sync ();
			private slots:
				void getNewDeltas ();
				void handleSuccess (const QList<QByteArray>&);
				void handleMaxDeltaIDReceived (quint32);
				void handleDeltasReceived (const QList<QByteArray>&);
				void handlePutDeltas ();
			signals:
				void gotNewDeltas (const Sync::Deltas_t&, const QByteArray&);
				void deltasRequired (Sync::Deltas_t*, const QByteArray&);

				// Used internally to control the statemachine.
				void initiated ();
				void hasNewDeltas ();
				void noNewDeltas ();
				void deltasReceived ();
				void deltasProcessed ();
				void fail ();
				void success ();
			};
		}
	}
}

#endif
