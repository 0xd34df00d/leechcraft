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

#include "azothclientconnection.h"
#include <stdexcept>
#include <QDBusPendingCallWatcher>
#include <QDBusReply>
#include <QtDebug>
#include "azothclientconnectionadaptor.h"
#include "core.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Azoth
		{
			namespace Server
			{
				AzothClientConnection::AzothClientConnection (QObject *parent)
				: QObject (parent)
				{
					new AzothClientConnectionAdaptor (this);

					QDBusConnection::sessionBus ().registerService ("org.LeechCraft.Azoth.Server");
					QDBusConnection::sessionBus ().registerObject ("/Azoth/Server", this);
				}

				void AzothClientConnection::Establish ()
				{
					qDebug () << Q_FUNC_INFO;
					Connection_.reset (new QDBusInterface ("org.LeechCraft.Azoth.Client",
								"/Azoth/Client"));
					if (!Connection_->isValid ())
					{
						qWarning () << Q_FUNC_INFO
							<< Connection_->lastError ();
						throw std::runtime_error (qPrintable (Connection_->
									lastError ().message ()));
					}

					QDBusReply<void> rep = Connection_->call ("ServerReady");
					if (!rep.isValid ())
					{
						QString errString = "ServerReady() call failure: " +
							rep.error ().name () + "; " +
							rep.error ().message ();
						qWarning () << Q_FUNC_INFO
							<< errString;
						throw std::runtime_error (qPrintable (errString));
					}
				}

				void AzothClientConnection::RequestPlugins ()
				{
					if (!Connection_.get () ||
							!Connection_->isValid ())
					{
						qWarning () << Q_FUNC_INFO
							<< "connection is invalid";
						throw std::runtime_error ("Invalid connection");
					}

					QDBusPendingCall pcall = Connection_->asyncCall ("ReaddProtocolPlugins");
					QDBusPendingCallWatcher *pw = new QDBusPendingCallWatcher (pcall, this);
					connect (pw,
							SIGNAL (finished (QDBusPendingCallWatcher*)),
							this,
							SLOT (handleReaddProtocolPluginsFinished (QDBusPendingCallWatcher*)));
				}

				bool AzothClientConnection::AddProtocolPluginByPath (const QString& path)
				{
					qDebug () << Q_FUNC_INFO << path;
					return true;
				}

				void AzothClientConnection::Shutdown ()
				{
					Core::Instance ().Shutdown ();
				}

				void AzothClientConnection::handleReaddProtocolPluginsFinished (QDBusPendingCallWatcher *w)
				{
					QDBusPendingReply<void> reply = *w;
					if (reply.isError ())
						qWarning () << Q_FUNC_INFO
							<< "ReaddProtocolPlugins() call failure"
							<< reply.error ().name ()
							<< reply.error ().message ();
				}
			};
		};
	};
};

