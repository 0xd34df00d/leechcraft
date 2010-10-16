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

#include "core.h"
#include <QCoreApplication>
#include <QTimer>
#include <plugininterface/util.h>
#include "datastorageserver.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Syncer
		{
			Core::Core ()
			: DataStorage_ (new DataStorageServer (this))
			, Settings_ (QCoreApplication::organizationName (),
					QCoreApplication::applicationName () + "_Syncer")
			{
				qRegisterMetaType<QList<QByteArray> > ("QList<QByteArray>");

				connect (DataStorage_,
						SIGNAL (gotNewDeltas (const Sync::Deltas_t&, const QByteArray&)),
						this,
						SLOT (handleNewDeltas (const Sync::Deltas_t&, const QByteArray&)));
				connect (DataStorage_,
						SIGNAL (deltasRequired (Sync::Deltas_t*, const QByteArray&)),
						this,
						SLOT (handleDeltasRequired (Sync::Deltas_t*, const QByteArray&)));
				connect (DataStorage_,
						SIGNAL (successfullySentDeltas (quint32, const QByteArray&)),
						this,
						SLOT (handleSuccessfullySentDeltas (quint32, const QByteArray&)));

				connect (DataStorage_,
						SIGNAL (loginError (const QByteArray&)),
						this,
						SLOT (handleLoginError (const QByteArray&)));
				connect (DataStorage_,
						SIGNAL (connectionError (const QByteArray&)),
						this,
						SLOT (handleConnectionError (const QByteArray&)));
				connect (DataStorage_,
						SIGNAL (finishedSuccessfully (quint32, quint32, const QByteArray&)),
						this,
						SLOT (handleFinishedSuccessfully (quint32, quint32, const QByteArray&)));
			}

			Core& Core::Instance ()
			{
				static Core c;
				return c;
			}

			void Core::SetProxy (ICoreProxy_ptr proxy)
			{
				Proxy_ = proxy;
			}

			void Core::SecondInit ()
			{
				QObjectList plugins = Proxy_->GetPluginsManager ()->
						GetAllCastableRoots<ISyncable*> ();
				Q_FOREACH (QObject *plugin, plugins)
					ID2Object_ [qobject_cast<IInfo*> (plugin)->GetUniqueID ()] = plugin;

				QTimer::singleShot (5000, this, SLOT (syncAll ()));
			}

			quint32 Core::GetLastID (const QByteArray& chain) const
			{
				qDebug () << Q_FUNC_INFO << chain;
				Settings_.beginGroup ("IDs");
				quint32 value = Settings_.value (chain, 0).value<quint32> ();
				Settings_.endGroup ();
				return value;
			}

			void Core::SetLastID (const QByteArray& chain, quint32 id)
			{
				qDebug () << Q_FUNC_INFO << chain;
				Settings_.beginGroup ("IDs");
				Settings_.setValue (chain, id);
				Settings_.endGroup ();
			}

			void Core::syncAll ()
			{
				QObjectList plugins = ID2Object_.values ();
				Q_FOREACH (QObject *plugin, ID2Object_.values ())
				{
					QByteArray id = qobject_cast<IInfo*> (plugin)->GetUniqueID ();
					ISyncable *syncable = qobject_cast<ISyncable*> (plugin);
					Sync::ChainIDs_t chains = syncable->AvailableChains ();
					Q_FOREACH (const Sync::ChainID_t& cid, chains)
					{
						QByteArray fullChain = id + "$" + cid;
						DataStorage_->sync (fullChain);
					}
				}
			}

			void Core::handleNewDeltas (const Sync::Deltas_t& deltas, const QByteArray& fullChain)
			{
				QList<QByteArray> parts = fullChain.split ('$');
				QByteArray pluginId = parts.at (0);
				QByteArray chain = parts.at (1);

				if (!ID2Object_.contains (pluginId))
				{
					qWarning () << Q_FUNC_INFO
							<< "unknown plugin ID"
							<< pluginId;
					return;
				}

				ISyncable *syncable = qobject_cast<ISyncable*> (ID2Object_ [pluginId]);

				Sync::Payloads_t payloads;
				Q_FOREACH (const Sync::Delta& delta, deltas)
					payloads << delta.Payload_;

				syncable->ApplyDeltas (payloads, chain);

				SetLastID (fullChain, GetLastID (fullChain) + payloads.size ());
			}

			void Core::handleDeltasRequired (Sync::Deltas_t *deltas, const QByteArray& fullChain)
			{
				QList<QByteArray> parts = fullChain.split ('$');
				QByteArray pluginId = parts.at (0);
				QByteArray chain = parts.at (1);

				if (!ID2Object_.contains (pluginId))
				{
					qWarning () << Q_FUNC_INFO
							<< "unknown plugin ID"
							<< pluginId;
					return;
				}

				ISyncable *syncable = qobject_cast<ISyncable*> (ID2Object_ [pluginId]);

				quint32 id = GetLastID (fullChain);
				Sync::Payloads_t payloads = id ?
						syncable->GetNewDeltas (chain) :
						syncable->GetAllDeltas (chain);
				Q_FOREACH (const Sync::Payload& payload, payloads)
				{
					Sync::Delta delta =
					{
						++id,
						payload
					};

					*deltas << delta;
				}
			}

			void Core::handleSuccessfullySentDeltas (quint32 numDeltas, const QByteArray& fullChain)
			{
				QList<QByteArray> parts = fullChain.split ('$');
				QByteArray pluginId = parts.at (0);
				QByteArray chain = parts.at (1);

				if (!ID2Object_.contains (pluginId))
				{
					qWarning () << Q_FUNC_INFO
							<< "unknown plugin ID"
							<< pluginId;
					return;
				}

				ISyncable *syncable = qobject_cast<ISyncable*> (ID2Object_ [pluginId]);

				syncable->PurgeNewDeltas (chain, numDeltas);
				SetLastID (fullChain, GetLastID (fullChain) + numDeltas);
			}

			void Core::handleLoginError (const QByteArray& chain)
			{
				QString name = GetNameForChain (chain);
				if (name.isEmpty ())
				{
					qWarning () << Q_FUNC_INFO
							<< "^^^^^^^^^^^^";
					return;
				}

				emit gotEntity (Util::MakeNotification (tr ("Sync failure"),
						tr ("Login error when synchronizing plugin %1.")
							.arg (name),
						PCritical_));
			}

			void Core::handleConnectionError (const QByteArray& chain)
			{
				QString name = GetNameForChain (chain);
				if (name.isEmpty ())
				{
					qWarning () << Q_FUNC_INFO
							<< "^^^^^^^^^^^^";
					return;
				}

				emit gotEntity (Util::MakeNotification (tr ("Sync failure"),
						tr ("Connection error when synchronizing plugin %1.")
							.arg (name),
						PCritical_));
			}

			void Core::handleFinishedSuccessfully (quint32 sent, quint32 received, const QByteArray& chain)
			{
				QString name = GetNameForChain (chain);
				if (name.isEmpty ())
				{
					qWarning () << Q_FUNC_INFO
							<< "^^^^^^^^^^^^";
					return;
				}

				if (sent + received)
				{
					QString text = tr ("Successfully synchronized plugin %1");
					text += ": ";
					text += tr ("%n item(s) received", 0, received);
					text += ", ";
					text += tr ("%n item(s) sent", 0, received);
					emit gotEntity (Util::MakeNotification (tr ("Sync"),
							text,
							PInfo_));
				}
			}

			QString Core::GetNameForChain (const QByteArray& fullChain)
			{
				QList<QByteArray> parts = fullChain.split ('$');
				QByteArray pluginId = parts.at (0);

				if (!ID2Object_.contains (pluginId))
				{
					qWarning () << Q_FUNC_INFO
							<< "unknown plugin ID"
							<< pluginId;
					return QString ();
				}

				return qobject_cast<IInfo*> (ID2Object_ [pluginId])->GetName ();
			}
		}
	}
}
