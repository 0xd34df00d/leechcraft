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

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_CLIENTCONNECTION_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_CLIENTCONNECTION_H
#include <boost/shared_ptr.hpp>
#include <QObject>
#include <QMap>
#include <gloox/connectionlistener.h>
#include <gloox/rosterlistener.h>

class QTimer;

namespace gloox
{
	class Client;
	class JID;
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
					struct GlooxAccountState;

					class GlooxAccount;
					class GlooxCLEntry;

					class ClientConnection : public QObject
										   , public gloox::ConnectionListener
										   , public gloox::RosterListener
					{
						Q_OBJECT

						boost::shared_ptr<gloox::Client> Client_;
						QTimer *PollTimer_;
						GlooxAccount *Account_;
						QMap<gloox::JID, GlooxCLEntry*> JID2CLEntry_;
					public:
						ClientConnection (const gloox::JID&,
								const QString&,
								const GlooxAccountState&,
								GlooxAccount*);

						void SetState (const GlooxAccountState&);
						void Synchronize ();
						gloox::Client* GetClient () const;
						GlooxCLEntry* GetCLEntry (const gloox::JID& bareJid) const;
					protected:
						// ConnectionListener
						virtual void onConnect ();
						virtual void onDisconnect (gloox::ConnectionError);
						virtual void onResourceBind (const std::string&);
						virtual void onResourceBindError (const gloox::Error*);
						virtual void onSessionCreateError (const gloox::Error*);
						virtual void onStreamEvent (gloox::StreamEvent);
						virtual bool onTLSConnect (const gloox::CertInfo&);

						// RosterListener
						virtual void handleItemAdded (const gloox::JID&);
						virtual void handleItemSubscribed (const gloox::JID&);
						virtual void handleItemRemoved (const gloox::JID&);
						virtual void handleItemUpdated (const gloox::JID&);
						virtual void handleItemUnsubscribed (const gloox::JID&);
						virtual void handleRoster (const gloox::Roster&);
						virtual void handleRosterPresence (const gloox::RosterItem&,
								const std::string&, gloox::Presence::PresenceType, const std::string&);
						virtual void handleSelfPresence (const gloox::RosterItem&,
								const std::string&, gloox::Presence::PresenceType, const std::string&);
						virtual bool handleSubscriptionRequest (const gloox::JID&, const std::string&);
						virtual bool handleUnsubscriptionRequest (const gloox::JID&, const std::string&);
						virtual void handleNonrosterPresence (const gloox::Presence&);
						virtual void handleRosterError (const gloox::IQ&);
					private slots:
						void handlePollTimer ();
					signals:
						void gotRosterItems (const QList<QObject*>&);
						void rosterItemRemoved (QObject*);
						void rosterItemUpdated (QObject*);
					};
				}
			}
		}
	}
}

#endif
