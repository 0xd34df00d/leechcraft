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
#include <gloox/rostermanager.h>
#include "glooxaccount.h"
#include "config.h"
#include "glooxclentry.h"

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
							const GlooxAccountState& state,
							GlooxAccount *account)
					: Account_ (account)
					{
						Client_.reset (new gloox::Client (jid, pwd.toUtf8 ().constData ()));

						Client_->registerConnectionListener (this);
						Client_->rosterManager ()->registerRosterListener (this, false);

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

					void ClientConnection::Synchronize ()
					{
						Client_->rosterManager ()->synchronize ();
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

					void ClientConnection::handleItemAdded (const gloox::JID& jid)
					{
						gloox::RosterItem *ri = Client_->rosterManager ()->getRosterItem (jid);

						GlooxCLEntry *entry = new GlooxCLEntry (ri, Account_);
						JID2CLEntry_ [jid] = entry;
						emit gotRosterItems (QList<QObject*> () << entry);
					}

					void ClientConnection::handleItemSubscribed (const gloox::JID& jid)
					{
						// TODO
					}

					void ClientConnection::handleItemRemoved (const gloox::JID& jid)
					{
						if (!JID2CLEntry_.contains (jid))
						{
							qWarning () << Q_FUNC_INFO
									<< "strange, we have no"
									<< jid.full ().c_str ()
									<< "in our JID2CLEntry_";
							return;
						}

						GlooxCLEntry *entry = JID2CLEntry_.take (jid);
						emit rosterItemRemoved (entry);
					}

					void ClientConnection::handleItemUpdated (const gloox::JID& jid)
					{
						if (!JID2CLEntry_.contains (jid))
						{
							qWarning () << Q_FUNC_INFO
									<< "strange, we have no"
									<< jid.full ().c_str ()
									<< "in our JID2CLEntry_";
							return;
						}

						emit rosterItemUpdated (JID2CLEntry_ [jid]);
					}

					void ClientConnection::handleItemUnsubscribed (const gloox::JID& jid)
					{
						// TODO
					}

					void ClientConnection::handleRoster (const gloox::Roster& roster)
					{
						QList<QObject*> entries;
						for (gloox::Roster::const_iterator i = roster.begin (),
								end = roster.end (); i != end; ++i)
						{
							GlooxCLEntry *entry = new GlooxCLEntry (i->second, Account_);
							entries << entry;

							JID2CLEntry_ [i->first] = entry;
						}

						if (entries.size ())
							emit gotRosterItems (entries);
					}

					void ClientConnection::handleRosterPresence (const gloox::RosterItem& item,
								const std::string& resource,
								gloox::Presence::PresenceType type,
								const std::string& msg)
					{
						// TODO
					}

					void ClientConnection::handleSelfPresence (const gloox::RosterItem& item,
								const std::string& resource,
								gloox::Presence::PresenceType type,
								const std::string& msg)
					{
						// TODO
					}

					bool ClientConnection::handleSubscriptionRequest (const gloox::JID&, const std::string&)
					{
						// TODO
						return false;
					}

					bool ClientConnection::handleUnsubscriptionRequest (const gloox::JID&, const std::string&)
					{
						// TODO
						return false;
					}

					void ClientConnection::handleNonrosterPresence (const gloox::Presence&)
					{
						// TODO
					}

					void ClientConnection::handleRosterError (const gloox::IQ&)
					{
						// TODO
					}
				}
			}
		}
	}
}
