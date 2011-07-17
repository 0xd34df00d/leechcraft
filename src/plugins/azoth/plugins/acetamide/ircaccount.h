/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011 Oleg Linkin
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

#ifndef PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCACCOUNT_H
#define PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCACCOUNT_H

#include <boost/shared_ptr.hpp>
#include <QObject>
#include <interfaces/iaccount.h>
#include <interfaces/imessage.h>
#include <interfaces/ihaveconsole.h>
#include "core.h"
#include "localtypes.h"

namespace LeechCraft
{
namespace Azoth
{

class IProtocol;

namespace Acetamide
{

	class ClientConnection;
	class IrcProtocol;
	class IrcAccountConfigurationWidget;

	class IrcAccount : public QObject
						, public IAccount
						, public IHaveConsole
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Azoth::IAccount LeechCraft::Azoth::IHaveConsole);

		QString AccountName_;
		IrcProtocol *ParentProtocol_;
		QByteArray AccountID_;

		QString RealName_;
		QString UserName_;
		QStringList NickNames_;
		State IrcAccountState_;

		boost::shared_ptr<ClientConnection> ClientConnection_;
	public:
		IrcAccount (const QString&, QObject*);
		void Init ();

		QObject* GetObject ();
		QObject* GetParentProtocol () const;
		AccountFeatures GetAccountFeatures () const;
		QList<QObject*> GetCLEntries ();
		void QueryInfo (const QString&);

		QString GetAccountName () const;
		QString GetOurNick () const;
		QString GetUserName () const;
		QString GetRealName () const;
		QStringList GetNickNames () const;

		boost::shared_ptr<ClientConnection> GetClientConnection () const;

		void RenameAccount (const QString&);

		QByteArray GetAccountID () const;
		void SetAccountID (const QString&);
		
		QList<QAction*> GetActions () const;

		void OpenConfigurationDialog ();
		void FillSettings (IrcAccountConfigurationWidget*);

		void JoinServer (const ServerOptions&, const ChannelOptions&);

		EntryStatus GetState () const;
		void ChangeState (const EntryStatus&);
		void Synchronize ();
		void Authorize (QObject*);
		void DenyAuth (QObject*);
		void RequestAuth (const QString&, const QString&,
				const QString&, const QStringList&);
		void RemoveEntry (QObject*);
		QObject* GetTransferManager () const;

		PacketFormat GetPacketFormat () const;
		void SetConsoleEnabled (bool);
	public:
		QByteArray Serialize () const;
		static IrcAccount* Deserialize (const QByteArray&, QObject*);
	public slots:
		void handleEntryRemoved (QObject*);
		void handleGotRosterItems (const QList<QObject*>&);
	private slots:
		void handleDestroyClient ();
	signals:
		void gotCLItems (const QList<QObject*>&);
		void removedCLItems (const QList<QObject*>&);
		void authorizationRequested (QObject*, const QString&);
		void itemSubscribed (QObject*, const QString&);
		void itemUnsubscribed (QObject*, const QString&);
		void itemUnsubscribed (const QString&, const QString&);
		void itemCancelledSubscription (QObject*, const QString&);
		void itemGrantedSubscription (QObject*, const QString&);
		void statusChanged (const EntryStatus&);
		void addContactSuggested (const QString&, const QString&, const QStringList&);

		void accountSettingsChanged ();

		void scheduleClientDestruction ();

		void gotConsolePacket (const QByteArray&, int);
	};

	typedef boost::shared_ptr<IrcAccount> IrcAccount_ptr;
};
};
};
#endif // PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCACCOUNT_H
