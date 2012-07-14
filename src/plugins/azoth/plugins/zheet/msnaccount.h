/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#ifndef PLUGINS_AZOTH_PLUGINS_ZHEET_MSNACCOUNT_H
#define PLUGINS_AZOTH_PLUGINS_ZHEET_MSNACCOUNT_H
#include <QObject>
#include <QSet>
#include <msn/passport.h>
#include <msn/util.h>
#include <interfaces/azoth/iaccount.h>
#include <interfaces/azoth/iextselfinfoaccount.h>

namespace MSN
{
	class NotificationServerConnection;
	class Buddy;
	class Message;
}

namespace LeechCraft
{
namespace Azoth
{
namespace Zheet
{
	class MSNProtocol;
	class Callbacks;
	class MSNAccountConfigWidget;
	class MSNBuddyEntry;
	class SBManager;
	class GroupManager;
	class TransferManager;

	class MSNAccount : public QObject
					 , public IAccount
					 , public IExtSelfInfoAccount
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Azoth::IAccount LeechCraft::Azoth::IExtSelfInfoAccount);

		MSNProtocol *Proto_;

		QString Name_;
		MSN::Passport Passport_;
		QString Server_;
		int Port_;

		Callbacks *CB_;
		MSN::NotificationServerConnection *Conn_;
		SBManager *SB_;
		GroupManager *GroupManager_;
		TransferManager *TM_;

		EntryStatus PendingStatus_;
		bool Connecting_;
		EntryStatus CurrentStatus_;

		QHash<QString, MSNBuddyEntry*> Entries_;
		QHash<QString, MSNBuddyEntry*> CID2Entry_;

		QSet<QString> BL_;

		QString OurFriendlyName_;

		QAction *ActionManageBL_;
	public:
		MSNAccount (const QString&, MSNProtocol* = 0);
		void Init ();

		QByteArray Serialize () const;
		static MSNAccount* Deserialize (const QByteArray&, MSNProtocol*);

		void FillConfig (MSNAccountConfigWidget*);

		MSN::NotificationServerConnection* GetNSConnection () const;
		SBManager* GetSBManager () const;
		GroupManager* GetGroupManager () const;

		MSNBuddyEntry* GetBuddy (const QString&) const;
		MSNBuddyEntry* GetBuddyByCID (const QString&) const;

		QSet<QString> GetBL () const;
		void RemoveFromBL (const QString&);

		// IAccount
		QObject* GetObject ();
		QObject* GetParentProtocol () const;
		AccountFeatures GetAccountFeatures () const;
		QList<QObject*> GetCLEntries ();
		QString GetAccountName () const;
		QString GetOurNick () const;
		void RenameAccount (const QString& name);
		QByteArray GetAccountID () const;
		QList<QAction*> GetActions () const;
		void QueryInfo (const QString&);
		void OpenConfigurationDialog ();
		EntryStatus GetState () const;
		void ChangeState (const EntryStatus&);
		void Authorize (QObject*);
		void DenyAuth (QObject*);
		void RequestAuth (const QString&, const QString&, const QString&, const QStringList&);
		void RemoveEntry (QObject*);
		QObject* GetTransferManager () const;

		// IExtSelfInfoAccount
		QObject* GetSelfContact () const;
		QImage GetSelfAvatar () const;
		QIcon GetAccountIcon () const;
	private slots:
		void handleConnected ();
		void handleWeChangedState (State);
		void handleGotOurFriendlyName (const QString&);
		void handleBuddyChangedStatus (const QString&, State);
		void handleGotBuddies (const QList<MSN::Buddy*>&);
		void handleRemovedBuddy (const QString&, const QString&);
		void handleRemovedBuddy (MSN::ContactList, const QString&);
		void handleGotMessage (const QString&, MSN::Message*);
		void handleGotNudge (const QString&);

		void handleInitialEmailNotification (int, int);
		void handleNewEmailNotification (const QString&, const QString&);

		void handleManageBL ();
	signals:
		void gotCLItems (const QList<QObject*>&);
		void removedCLItems (const QList<QObject*>&);
		void accountRenamed (const QString&);
		void authorizationRequested (QObject*, const QString&);
		void itemSubscribed (QObject*, const QString&);
		void itemUnsubscribed (QObject*, const QString&);
		void itemUnsubscribed (const QString&, const QString&);
		void itemCancelledSubscription (QObject*, const QString&);
		void itemGrantedSubscription (QObject*, const QString&);
		void statusChanged (const EntryStatus&);
		void mucInvitationReceived (const QVariantMap&,
				const QString&, const QString&);

		void accountSettingsChanged ();
	};
}
}
}

#endif
