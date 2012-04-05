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

#pragma once

#include <QObject>
#include <QImage>
#include <interfaces/azoth/iaccount.h>
#include <interfaces/azoth/isupporttune.h>
#include <interfaces/azoth/iextselfinfoaccount.h>
#include "proto/contactinfo.h"
#include "proto/headers.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Vader
{
	namespace Proto
	{
		class Connection;
		struct Message;
	}

	class MRIMProtocol;
	class MRIMAccountConfigWidget;
	class MRIMBuddy;
	class GroupManager;
	class SelfAvatarFetcher;

	class MRIMAccount : public QObject
					  , public IAccount
					  , public ISupportTune
					  , public IExtSelfInfoAccount
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Azoth::IAccount
				LeechCraft::Azoth::ISupportTune
				LeechCraft::Azoth::IExtSelfInfoAccount);

		MRIMProtocol *Proto_;
		QString Name_;
		QString Login_;

		Proto::Connection *Conn_;
		GroupManager *GM_;
		SelfAvatarFetcher *AvatarFetcher_;

		EntryStatus Status_;
		QHash<QString, MRIMBuddy*> Buddies_;
		QHash<quint32, Proto::ContactInfo> PendingAdditions_;

		QList<QAction*> Actions_;

		QImage SelfAvatar_;
	public:
		MRIMAccount (const QString&, MRIMProtocol*);

		void FillConfig (MRIMAccountConfigWidget*);
		Proto::Connection* GetConnection () const;
		GroupManager* GetGroupManager () const;
		void SetTypingState (const QString&, ChatPartState);
		void RequestInfo (const QString&);

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

		// ISupportTune
		void PublishTune (const QMap<QString, QVariant>&);

		// IExtSelfInfoAccount
		QObject* GetSelfContact () const;
		QImage GetSelfAvatar () const;
		QIcon GetAccountIcon () const;

		QByteArray Serialize () const;
		static MRIMAccount* Deserialize (const QByteArray&, MRIMProtocol*);
	private:
		MRIMBuddy* GetBuddy (const Proto::ContactInfo&);
	private slots:
		void updateSelfAvatar (const QImage&);

		void handleGotContacts (const QList<Proto::ContactInfo>&);
		void handleUserStatusChanged (const Proto::ContactInfo&);
		void handleContactAdded (quint32, quint32);

		void handleGotUserInfoError (const QString&, Proto::AnketaInfoStatus);
		void handleGotUserInfo (const QString&, const QMap<QString, QString>&);

		void handleGotAuthRequest (const QString&, const QString&);
		void handleGotAuthAck (const QString&);

		void handleGotMessage (const Proto::Message&);
		void handleGotAttentionRequest (const QString&, const QString&);
		void handleOurStatusChanged (const EntryStatus&);
		void handleGotUserTune (const QString&, const QString&);
		void handleUserStartedTyping (const QString&);
		void handleUserStoppedTyping (const QString&);

		void handleGotNewMail (const QString&, const QString&);
		void handleGotPOPKey (const QString&);

		void handleOpenMailbox ();
		void handleServices ();

		void handleShowTechSupport ();
	signals:
		void gotCLItems (const QList<QObject*>&);
		void removedCLItems (const QList<QObject*>&);
		void authorizationRequested (QObject*, const QString&);
		void accountRenamed (const QString&);
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
