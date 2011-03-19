/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Oleg Linkin
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
#include "core.h"

namespace LeechCraft
{
namespace Azoth
{
class IProtocol;
namespace Acetamide
{
	class IrcProtocol;
	class ClientConnection;
	
	class IrcAccount : public QObject
						, public IAccount
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Azoth::IAccount);
		
		QString AccountName_;
		IrcProtocol *ParentProtocol_;
		QByteArray AccountID_;

		QList<ServerOptions> Servers_;
		QList<ChannelOptions> Channels_;
		
		boost::shared_ptr<ClientConnection> ClientConnection_;
		State IrcAccountState;
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
		QList<ServerOptions> GetServers () const;
		QList<ChannelOptions> GetChannels () const;

		void RenameAccount (const QString&);
		
		QByteArray GetAccountID () const;
		void OpenConfigurationDialog ();
		EntryStatus GetState () const;
		void ChangeState (const EntryStatus&);
		void Synchronize ();
		void Authorize (QObject*);
		void DenyAuth (QObject*);
		void RequestAuth (const QString&, const QString&,
				const QString&, const QStringList&);
		void RemoveEntry (QObject*);
		QObject* GetTransferManager () const;
		void JoinRoom (const ServerOptions&, const ChannelOptions&);
		boost::shared_ptr<ClientConnection> GetClientConnection () const;
		QObject* CreateMessage (IMessage::MessageType,
				const QString&, const QString&);
	private:
		void SetAccountID ();
	public:
		QByteArray Serialize () const;
		static IrcAccount* Deserialize (const QByteArray&, QObject*);
		void SaveServersSettings (const QList<ServerOptions>&, const QString&);
		QList<ServerOptions> ReadServersSettings (const QString&) const;
		void SaveChannelsSettings (const QList<ChannelOptions>&, const QString&);
		QList<ChannelOptions> ReadChannelsSettings (const QString&) const;
	public slots:
		void handleEntryRemoved (QObject*);
		void handleGotRosterItems (const QList<QObject*>&);
	private slots:
		void handleDestroyClient ();
	signals:
		void gotCLItems (const QList<QObject*>&);
		void removedCLItems (const QList<QObject*>&);
		void joinedGroupchat (QObject*);
		void authorizationRequested (QObject*, const QString&);
		void itemSubscribed (QObject*, const QString&);
		void itemUnsubscribed (QObject*, const QString&);
		void itemUnsubscribed (const QString&, const QString&);
		void itemCancelledSubscription (QObject*, const QString&);
		void itemGrantedSubscription (QObject*, const QString&);
		void statusChanged (const EntryStatus&);

		void accountSettingsChanged ();

		void scheduleClientDestruction ();
	};
	
	typedef boost::shared_ptr<IrcAccount> IrcAccount_ptr;
	QDataStream& operator<< (QDataStream& out, const ServerOptions&);
	QDataStream& operator>> (QDataStream& in, ServerOptions&);
	
	QDataStream& operator<< (QDataStream& out, const ChannelOptions&);
	QDataStream& operator>> (QDataStream& in, ChannelOptions&);
};
};
};
#endif // PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCACCOUNT_H
