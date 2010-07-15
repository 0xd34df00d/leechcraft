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

#include "clientconnection.h"
#include <QTimer>
#include <QtDebug>
#include <gloox/presence.h>
#include <gloox/client.h>
#include <gloox/message.h>
#include <gloox/messagesession.h>
#include <gloox/error.h>
#include <gloox/capabilities.h>
#include "glooxaccount.h"
#include "config.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Azoth
		{
			namespace Plugins
			{
				namespace Xoox
				{
					ClientConnection::ClientConnection (const gloox::JID& jid,
							const QString& pwd,
							const GlooxAccountState& state)
					{
						Client_.reset (new gloox::Client (jid, pwd.toUtf8 ().constData ()));

						Client_->registerConnectionListener (this);
						//Client_->rosterManager ()->registerRosterListener (this, false);

						gloox::Capabilities *caps = new gloox::Capabilities (Client_->disco ());
						caps->setNode ("http://leechcraft.org/azoth");
						Client_->addPresenceExtension (caps);

						Client_->disco ()->setVersion ("LeechCraft Azoth", LEECHCRAFT_VERSION, "Gentŏŏ Linux");
						Client_->disco ()->setIdentity ("client", "pc", "LeechCraft Azoth");
						Client_->disco ()->addFeature ("jabber:iq:roster");

						SetState (state);

						QTimer *pollTimer = new QTimer (this);
						connect (pollTimer,
								SIGNAL (timeout ()),
								this,
								SLOT (handlePollTimer ()));
						pollTimer->start (50);

						Client_->connect (false);
					}

					void ClientConnection::SetState (const GlooxAccountState& state)
					{
						gloox::Presence::PresenceType pres = state.State_ > 0 ?
								static_cast<gloox::Presence::PresenceType> (state.State_ - 1) :
								gloox::Presence::Invalid;
						Client_->setPresence (pres, state.Priority_,
								state.Status_.toUtf8 ().constData ());
					}

					void ClientConnection::onConnect ()
					{
						qDebug () << Q_FUNC_INFO;
					}

					void ClientConnection::onDisconnect (gloox::ConnectionError e)
					{
						qWarning () << Q_FUNC_INFO << e << Client_->streamErrorText ().c_str () << Client_->authError ();
					}

					void ClientConnection::onResourceBind (const std::string& resource)
					{
						qDebug () << Q_FUNC_INFO << resource.c_str ();
					}

					void ClientConnection::onResourceBindError (const gloox::Error *error)
					{
						qWarning () << Q_FUNC_INFO;
						if (error)
							qWarning () << error->text ().c_str ();
					}

					void ClientConnection::onSessionCreateError (const gloox::Error *error)
					{
						qWarning () << Q_FUNC_INFO;
						if (error)
							qWarning () << error->text ().c_str ();
					}

					void ClientConnection::onStreamEvent (gloox::StreamEvent e)
					{
						qDebug () << Q_FUNC_INFO;
					}

					bool ClientConnection::onTLSConnect (const gloox::CertInfo& info)
					{
						qDebug () << Q_FUNC_INFO << info.server.c_str ();
						return true;
					}

					void ClientConnection::handlePollTimer ()
					{
						Client_->recv (1000);
					}
				}
			}
		}
	}
}
