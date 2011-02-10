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
#include <QMap>
#include <QVariant>
#include <interfaces/iaccount.h>
#include <interfaces/imessage.h>

namespace LeechCraft
{
namespace Azoth
{
class IProtocol;
namespace Acetamide
{
	class IrcProtocol;
	
	class IrcAccount : public QObject
						, public IAccount
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Azoth::IAccount);
		
		QString Name_;
		IrcProtocol *ParentProtocol_;
		State IrcAccountState_;
		
		QString Nicks_;
		QMap<QString, QVariant> Nicknames_;
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
// 		QString GetHost () const;
// 		int GetPort () const;
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

// 		QString GetJID () const;
// 		QString GetNick () const;
// 		void JoinRoom (const QString&, const QString&, const QString&);
// 		boost::shared_ptr<ClientConnection> GetClientConnection () const;
// 		GlooxCLEntry* CreateFromODS (GlooxCLEntry::OfflineDataSource_ptr);

		QByteArray Serialize () const;
		static IrcAccount* Deserialize (const QByteArray&, QObject*);

// 		QObject* CreateMessage (IMessage::MessageType,
// 				const QString&, const QString&,
// 				const QXmppRosterIq::Item&);
	private:
// 		QString GetPassword (bool authFailure = false);
		void SaveConnectionSettings (const QList<QVariant>&, const QString&);
		QList<QVariant> ReadConnectionSettings (const QString&);
// 	public slots:
// 		void handleEntryRemoved (QObject*);
// 		void handleGotRosterItems (const QList<QObject*>&);
	private slots:
// 		void handleServerAuthFailed ();
// 		void feedClientPassword ();
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
};
};
};
#endif // PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCACCOUNT_H
