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

					class ClientConnection : public QObject
										   , public gloox::ConnectionListener
					{
						Q_OBJECT

						boost::shared_ptr<gloox::Client> Client_;
						QTimer *PollTimer_;
					public:
						ClientConnection (const gloox::JID&,
								const QString&,
								const GlooxAccountState&);

						void SetState (const GlooxAccountState&);
					protected:
						// ConnectionListener
						virtual void onConnect ();
						virtual void onDisconnect (gloox::ConnectionError);
						virtual void onResourceBind (const std::string&);
						virtual void onResourceBindError (const gloox::Error*);
						virtual void onSessionCreateError (const gloox::Error*);
						virtual void onStreamEvent (gloox::StreamEvent);
						virtual bool onTLSConnect (const gloox::CertInfo&);
					private slots:
						void handlePollTimer ();
					};
				}
			}
		}
	}
}

#endif
