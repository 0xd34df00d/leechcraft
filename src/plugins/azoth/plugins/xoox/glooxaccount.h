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

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_GLOOXACCOUNT_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_GLOOXACCOUNT_H
#include <boost/shared_ptr.hpp>
#include <QObject>
#include <gloox/messagehandler.h>
#include <gloox/connectionlistener.h>
#include <interfaces/iaccount.h>

namespace gloox
{
	class Client;
}

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Azoth
		{
			namespace Plugins
			{
				class IProtocol;

				namespace Xoox
				{
					class ClientConnection;

					class GlooxAccount : public QObject
									   , public IAccount
									   , public gloox::MessageHandler
									   , public gloox::ConnectionListener
					{
						Q_OBJECT
						Q_INTERFACES (LeechCraft::Plugins::Azoth::Plugins::IAccount);

						QString Name_;
						IProtocol *ParentProtocol_;

						QString JID_;
						QString Nick_;
						QString Resource_;
						qint16 Priority_;

						boost::shared_ptr<gloox::Client> Client_;

						friend class AccountMessageHandler;
					public:
						GlooxAccount (const QString&, QObject*);

						QObject* GetObject ();
						IProtocol* GetParentProtocol () const;
						AccountFeatures GetAccountFeatures () const;
						QList<ICLEntry*> GetCLEntries ();
						QString GetAccountName () const;
						void RenameAccount (const QString&);
						QByteArray GetAccountID () const;
						void OpenConfigurationDialog ();
						void ChangeState (State, const QString& = QString ());
					protected:
						virtual void handleMessage (const gloox::Message&,
								gloox::MessageSession*);
						virtual void onConnect ();
						virtual void onDisconnect (gloox::ConnectionError);
						virtual void onResourceBind (const std::string&);
						virtual void onResourceBindError (const gloox::Error*);
						virtual void onSessionCreateError (const gloox::Error*);
						virtual void onStreamEvent (gloox::StreamEvent);
						virtual bool onTLSConnect (const gloox::CertInfo&);
					};
				}
			}
		}
	}
}

#endif
