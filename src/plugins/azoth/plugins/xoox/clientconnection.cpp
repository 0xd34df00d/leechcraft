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
#include <gloox/mucroom.h>
#include <plugininterface/util.h>
#include <interfaces/iprotocol.h>
#include "glooxaccount.h"
#include "config.h"
#include "glooxclentry.h"
#include "glooxmessage.h"
#include "roomhandler.h"
#include "glooxprotocol.h"

uint gloox::qHash (const gloox::JID& jid)
{
	return qHash (QByteArray (jid.full ().c_str ()));
}

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
						connect (this,
								SIGNAL (gotEntity (const LeechCraft::Entity&)),
								Account_->GetParentProtocol ()->GetObject (),
								SIGNAL (gotEntity (const LeechCraft::Entity&)));
						Client_.reset (new gloox::Client (jid, pwd.toUtf8 ().constData ()));

						Client_->registerMessageSessionHandler (this);

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
						gloox::Presence::PresenceType pres =
								static_cast<gloox::Presence::PresenceType> (state.State_);
						Client_->setPresence (pres, state.Priority_,
								state.Status_.toUtf8 ().constData ());
					}

					void ClientConnection::Synchronize ()
					{
						Client_->rosterManager ()->synchronize ();
					}

					RoomCLEntry* ClientConnection::JoinRoom (const gloox::JID& jid)
					{
						RoomHandler *rh = new RoomHandler (Account_);
						gloox::MUCRoom *room = new gloox::MUCRoom (Client_.get (), jid, rh, 0);
						room->join ();
						rh->SetRoom (room);

						return rh->GetCLEntry ();
					}

					gloox::Client* ClientConnection::GetClient () const
					{
						return Client_.get ();
					}

					GlooxCLEntry* ClientConnection::GetCLEntry (const gloox::JID& bareJid) const
					{
						return JID2CLEntry_ [bareJid];
					}

					GlooxMessage* ClientConnection::CreateMessage (IMessage::MessageType type,
							const QString& variant, const QString& body, gloox::RosterItem *ri)
					{
						gloox::JID jid = gloox::JID (ri->jid ());
						gloox::JID bareJid = jid.bareJID ();
						if (!Sessions_ [bareJid].contains (variant))
						{
							const std::string resource = variant.toUtf8 ().constData ();
							if (ri->resource (resource))
								jid.setResource (resource);

							gloox::MessageSession *ses =
									new gloox::MessageSession (Client_.get (), jid);
							ses->registerMessageHandler (this);
							Sessions_ [bareJid] [variant] = ses;
						}

						GlooxMessage *msg = new GlooxMessage (type, IMessage::DOut,
								JID2CLEntry_ [bareJid],
								Sessions_ [bareJid] [variant]);
						msg->SetBody (body);
						msg->SetDateTime (QDateTime::currentDateTime ());
						return msg;
					}

					void ClientConnection::onConnect ()
					{
						qDebug () << Q_FUNC_INFO;
					}

					void ClientConnection::onDisconnect (gloox::ConnectionError e)
					{
						if (e == gloox::ConnNoError)
							return;

						QString error;
						switch (e)
						{
						case gloox::ConnStreamError:
							error = tr ("stream error: %1")
									.arg (QString::fromUtf8 (Client_->
													streamErrorText ().c_str ()));
							break;
						case gloox::ConnStreamVersionError:
							error = tr ("stream version not supported");
							break;
						case gloox::ConnStreamClosed:
							error = tr ("stream has been closed by the server");
							break;
						case gloox::ConnProxyAuthRequired:
							error = tr ("proxy server requires authentication");
							break;
						case gloox::ConnProxyAuthFailed:
							error = tr ("proxy server authentication failed");
							break;
						case gloox::ConnProxyNoSupportedAuth:
							error = tr ("proxy server requires an unsupported authentication method");
							break;
						case gloox::ConnIoError:
							error = tr ("an I/O error occured");
							break;
						case gloox::ConnParseError:
							error = tr ("XML parse error occured");
							break;
						case gloox::ConnConnectionRefused:
							error = tr ("connection was refused by the server");
							break;
						case gloox::ConnDnsError:
							error = tr ("resolving the server's hostname failed");
							break;
						case gloox::ConnOutOfMemory:
							error = tr ("out of memory");
							break;
						case gloox::ConnNoSupportedAuth:
							error = tr ("authentication mechanisms offered by the server are not supported");
							break;
						case gloox::ConnTlsFailed:
							error = tr ("server's certificate could not be verified or TLS handshake failed");
							break;
						case gloox::ConnTlsNotAvailable:
							error = tr ("server didn't offer TLS");
							break;
						case gloox::ConnCompressionFailed:
							error = tr ("initializing compression failed");
							break;
						case gloox::ConnAuthenticationFailed:
							error = tr ("authentication failed, %1");
							break;
						case gloox::ConnUserDisconnected:
							error = tr ("user disconnect requested");
							break;
						case gloox::ConnNotConnected:
							error = tr ("no active connection");
							break;
						}

						if (e == gloox::ConnAuthenticationFailed)
						{
							QString ae;
							switch (Client_->authError ())
							{
								case gloox::AuthErrorUndefined:
									ae = tr ("error condition is unknown");
									break;
								case gloox::SaslAborted:
									ae = tr ("SASL aborted");
									break;
								case gloox::SaslIncorrectEncoding:
									ae = tr ("incorrect encoding");
									break;
								case gloox::SaslInvalidAuthzid:
									ae = tr ("authzid provided by initiating entity is invalid");
									break;
								case gloox::SaslInvalidMechanism:
									ae = tr ("initiating entity provided a mechanism not supported by the receiving entity");
									break;
								case gloox::SaslMalformedRequest:
									ae = tr ("malformed request");
									break;
								case gloox::SaslMechanismTooWeak:
									ae = tr ("mechanism requested by initiating entity is weaker than server policy permits");
									break;
								case gloox::SaslNotAuthorized:
								case gloox::NonSaslNotAuthorized:
									ae = tr ("initiating entity did not provide valid credentials");
									break;
								case gloox::SaslTemporaryAuthFailure:
									ae = tr ("temporary error withing receiving entity");
									break;
								case gloox::NonSaslConflict:
									ae = tr ("resource conflict");
									break;
								case gloox::NonSaslNotAcceptable:
									ae = tr ("required information not provided");
									break;
							}
							error = error.arg (ae);
						}

						QString message = tr ("Disconnected, %1.")
								.arg (error);
						qWarning () << Q_FUNC_INFO << message;

						Entity e = Util::MakeNotification (tr ("Azoth connection error"),
								message,
								PCritical_);
						emit gotEntity (e);
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
						/** @todo show a dialog about certificate and
						 * whether it should be accepted.
						 */
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

						GlooxCLEntry *entry = CreateCLEntry (ri);
						emit gotRosterItems (QList<QObject*> () << entry);
					}

					void ClientConnection::handleItemSubscribed (const gloox::JID& jid)
					{
						// TODO
					}

					void ClientConnection::handleItemRemoved (const gloox::JID& jid)
					{
						if (!JID2CLEntry_.contains (jid.bareJID ()))
						{
							qWarning () << Q_FUNC_INFO
									<< "strange, we have no"
									<< jid.full ().c_str ()
									<< "in our JID2CLEntry_";
							return;
						}

						GlooxCLEntry *entry = JID2CLEntry_.take (jid.bareJID ());
						emit rosterItemRemoved (entry);
					}

					void ClientConnection::handleItemUpdated (const gloox::JID& jid)
					{
						if (!JID2CLEntry_.contains (jid.bareJID ()))
						{
							qWarning () << Q_FUNC_INFO
									<< "strange, we have no"
									<< jid.full ().c_str ()
									<< "in our JID2CLEntry_";
							return;
						}

						emit rosterItemUpdated (JID2CLEntry_ [jid.bareJID ()]);
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
							GlooxCLEntry *entry = CreateCLEntry (i->second);
							entries << entry;
						}

						if (entries.size ())
							emit gotRosterItems (entries);
					}

					void ClientConnection::handleRosterPresence (const gloox::RosterItem& item,
								const std::string& resource,
								gloox::Presence::PresenceType type,
								const std::string& msg)
					{
						gloox::JID jid (item.jid ());
						if (!JID2CLEntry_.contains (jid))
						{
							qWarning () << Q_FUNC_INFO
									<< "no GlooxCLEntry for item"
									<< item.jid ().c_str ();
							return;
						}

						GlooxCLEntry *entry = JID2CLEntry_ [jid];

						EntryStatus status (static_cast<State> (type),
								QString::fromUtf8 (msg.c_str ()));

						entry->SetStatus (status);
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

					void ClientConnection::handleMessageSession (gloox::MessageSession *session)
					{
						gloox::JID jid = session->target ();
						gloox::JID bareJid = jid.bareJID ();
						QString resource = QString::fromUtf8 (jid.resource ().c_str ());
						if (!Sessions_ [bareJid].contains (resource))
							Sessions_ [bareJid] [resource] = session;

						session->registerMessageHandler (this);
					}

					void ClientConnection::handleMessage (const gloox::Message& msg, gloox::MessageSession *session)
					{
						gloox::JID jid = session->target ().bareJID ();

						if (!JID2CLEntry_.contains (jid))
						{
							qWarning () << Q_FUNC_INFO
									<< "map doesn't contain"
									<< QString::fromUtf8 (jid.full ().c_str());
							return;
						}

						GlooxCLEntry *entry = JID2CLEntry_ [jid];
						GlooxMessage *gm = new GlooxMessage (msg, entry, session);
						gm->SetDateTime (QDateTime::currentDateTime ());

						entry->ReemitMessage (gm);
					}

					GlooxCLEntry* ClientConnection::CreateCLEntry (gloox::RosterItem *ri)
					{
						GlooxCLEntry *entry = new GlooxCLEntry (ri, Account_);
						gloox::JID jid (ri->jid ());
						JID2CLEntry_ [jid.bareJID ()] = entry;
						return entry;
					}
				}
			}
		}
	}
}
