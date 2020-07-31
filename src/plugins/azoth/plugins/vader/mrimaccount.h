/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QImage>
#include <interfaces/azoth/iaccount.h>
#include <interfaces/azoth/isupporttune.h>
#include <interfaces/azoth/iextselfinfoaccount.h>
#include "proto/contactinfo.h"
#include "proto/headers.h"
#include "mrimprotocol.h"

namespace LC
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

	class MRIMAccountConfigWidget;
	class MRIMBuddy;
	class GroupManager;

	class MRIMAccount : public QObject
					  , public IAccount
					  , public ISupportTune
					  , public IExtSelfInfoAccount
	{
		Q_OBJECT
		Q_INTERFACES (LC::Azoth::IAccount
				LC::Azoth::ISupportTune
				LC::Azoth::IExtSelfInfoAccount)

		MRIMProtocol *Proto_;
		QString Name_;
		QString Login_;

		Proto::Connection *Conn_;
		GroupManager *GM_;

		EntryStatus Status_;
		QHash<QString, MRIMBuddy*> Buddies_;
		QHash<quint32, Proto::ContactInfo> PendingAdditions_;

		QList<QAction*> Actions_;
	public:
		MRIMAccount (const QString&, MRIMProtocol*);

		void FillConfig (MRIMAccountConfigWidget*);
		Proto::Connection* GetConnection () const;
		GroupManager* GetGroupManager () const;
		void SetTypingState (const QString&, ChatPartState);
		void RequestInfo (const QString&);

		// IAccount
		QObject* GetQObject ();
		MRIMProtocol* GetParentProtocol () const;
		AccountFeatures GetAccountFeatures () const;
		QList<QObject*> GetCLEntries ();
		QString GetAccountName () const;
		QString GetOurNick () const;
		void RenameAccount (const QString& name);
		QByteArray GetAccountID () const;
		QList<QAction*> GetActions () const;
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
		QIcon GetAccountIcon () const;

		QByteArray Serialize () const;
		static MRIMAccount* Deserialize (const QByteArray&, MRIMProtocol*);
	private:
		MRIMBuddy* GetBuddy (const Proto::ContactInfo&);
	private slots:
		void handleAuthError (const QString&);

		void handleGotContacts (const QList<Proto::ContactInfo>&);
		void handleUserStatusChanged (const Proto::ContactInfo&);
		void handleContactAdded (quint32, quint32);
		void handleContactAdditionError (quint32, Proto::ContactAck);

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
