/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Georg Rudoy
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

#include "azothserverconnection.h"
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusPendingCallWatcher>
#include <QtDebug>
#include <plugininterface/util.h>
#include "core.h"
#include "azothserverconnectionadaptor.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Azoth
		{
			AzothServerConnection::AzothServerConnection (QObject *parent)
			: QObject (parent)
			{
				new AzothServerConnectionAdaptor (this);

				QDBusConnection::sessionBus ().registerService ("org.LeechCraft.Azoth.Client");
				QDBusConnection::sessionBus ().registerObject ("/Azoth/Client", this);
			}

			void AzothServerConnection::Establish ()
			{
				if (!QDBusConnection::sessionBus ().interface ()->
						isServiceRegistered ("org.LeechCraft.Azoth.Server"))
				{
					QProcess *process = new QProcess ();
					connect (process,
							SIGNAL (error (QProcess::ProcessError)),
							this,
							SLOT (handleProcessError (QProcess::ProcessError)));
					process->start ("leechcraft_azoth_server");
				}
				else
					ServerReady ();
			}

			void AzothServerConnection::Release ()
			{
				if (Connection_.get ())
					Connection_->asyncCall ("Shutdown");
			}

			void AzothServerConnection::ServerReady ()
			{
				qDebug () << Q_FUNC_INFO;
				Connection_.reset (new QDBusInterface ("org.LeechCraft.Azoth.Server",
							"/Azoth/Server"));
				if (!Connection_->isValid ())
				{
					qWarning () << Q_FUNC_INFO
						<< Connection_->lastError ();
					return;
				}
			}

			void AzothServerConnection::ReaddProtocolPlugins ()
			{
				qDebug () << Q_FUNC_INFO;
				IPluginsManager *pm = Core::Instance ().GetProxy ()->GetPluginsManager ();
				QObjectList pps = Core::Instance ().GetProtocolPlugins ();
				Q_FOREACH (QObject *protocol, pps)
				{
					QString path = pm->GetPluginLibraryPath (protocol);
					QDBusPendingCall pending = Connection_->
						asyncCall ("AddProtocolPluginByPath", path);
					QDBusPendingCallWatcher *watcher =
						new QDBusPendingCallWatcher (pending, this);
					connect (watcher,
							SIGNAL (finished (QDBusPendingCallWatcher*)),
							this,
							SLOT (handleAddProtocolPluginCallFinished (QDBusPendingCallWatcher*)));
				}
			}

			void AzothServerConnection::handleProcessError (QProcess::ProcessError pe)
			{
				qWarning () << Q_FUNC_INFO
					<< "error"
					<< pe;

				QString text = tr ("Helper process error: %1").arg (pe);
				emit gotEntity (Util::MakeNotification ("Azoth", text, PCritical_));
			}

			void AzothServerConnection::handleAddProtocolPluginCallFinished (QDBusPendingCallWatcher *w)
			{
				QDBusPendingReply<bool> reply = *w;
				if (reply.isError ())
				{
					qWarning () << Q_FUNC_INFO
						<< reply.error ().name ()
						<< reply.error ().message ();
					return;
				}
				else if (!reply.argumentAt<0> ())
					qWarning () << Q_FUNC_INFO
						<< "server reports failure";

				w->deleteLater ();
			}
		};
	};
};

