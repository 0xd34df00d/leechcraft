/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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
#include <QMap>
#include <gloox/messagehandler.h>
#include <gloox/jid.h>
#include <interfaces/iaccount.h>
#include <interfaces/imessage.h>
#include "glooxclentry.h"

namespace gloox
{
	class Client;
	class RosterItem;
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

	struct GlooxAccountState
	{
		State State_;
		QString Status_;
		int Priority_;
	};

	class GlooxProtocol;

	class GlooxAccount : public QObject
					   , public IAccount
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Plugins::Azoth::Plugins::IAccount);

		QString Name_;
		GlooxProtocol *ParentProtocol_;

		QString JID_;
		QString Nick_;
		QString Resource_;

		boost::shared_ptr<ClientConnection> ClientConnection_;

		GlooxAccountState AccState_;
	public:
		GlooxAccount (const QString&, QObject*);
		void Init ();

		QObject* GetObject ();
		QObject* GetParentProtocol () const;
		AccountFeatures GetAccountFeatures () const;
		QList<QObject*> GetCLEntries ();
		QString GetAccountName () const;
		QString GetOurNick () const;
		void RenameAccount (const QString&);
		QByteArray GetAccountID () const;
		void QueryInfo (const QString&);
		void OpenConfigurationDialog ();
		EntryStatus GetState () const;
		void ChangeState (const EntryStatus&);
		void Synchronize ();
		void Authorize (QObject*);
		void DenyAuth (QObject*);
		void RequestAuth (const QString&, const QString&,
				const QString&, const QStringList&);

		QString GetJID () const;
		QString GetNick () const;
		void JoinRoom (const QString&, const QString&, const QString&);
		boost::shared_ptr<ClientConnection> GetClientConnection () const;
		GlooxCLEntry* CreateFromODS (GlooxCLEntry::OfflineDataSource_ptr);

		QByteArray Serialize () const;
		static GlooxAccount* Deserialize (const QByteArray&, QObject*);

		QObject* CreateMessage (IMessage::MessageType,
				const QString&, const QString&,
				gloox::RosterItem*);
	private:
		QString GetPassword (bool authFailure = false);
	public slots:
		void handleEntryRemoved (QObject*);
		void handleGotRosterItems (const QList<QObject*>&);
	private slots:
		void handleServerAuthFailed ();
		void feedClientPassword ();
		void handleDestroyClient ();
	signals:
		void gotCLItems (const QList<QObject*>&);
		void removedCLItems (const QList<QObject*>&);
		void joinedGroupchat (QObject*);
		void authorizationRequested (QObject*, const QString&);

		void accountSettingsChanged ();

		void scheduleClientDestruction ();
	};

	typedef boost::shared_ptr<GlooxAccount> GlooxAccount_ptr;
}
}
}
}
}

#endif
