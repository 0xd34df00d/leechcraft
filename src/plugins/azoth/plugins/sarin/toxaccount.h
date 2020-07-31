/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>
#include <util/threads/concurrentexception.h>
#include <interfaces/azoth/iaccount.h>
#include <interfaces/azoth/isupportmediacalls.h>
#include "toxaccountconfiguration.h"

namespace LC
{
namespace Azoth
{
namespace Sarin
{
	class ToxProtocol;
	class ToxThread;
	class ToxContact;
	class ChatMessage;
	class MessagesManager;
	class AccountConfigDialog;
	class FileTransferManager;

	class ToxAccount : public QObject
					 , public IAccount
					 , public ISupportMediaCalls
	{
		Q_OBJECT
		Q_INTERFACES (LC::Azoth::IAccount LC::Azoth::ISupportMediaCalls)

		ToxProtocol * const Proto_;
		const QByteArray UID_;

		QString Name_;
		QString Nick_;

		ToxAccountConfiguration ToxConfig_;

		QByteArray ToxState_;

		QAction * const ActionGetToxId_;

		std::shared_ptr<ToxThread> Thread_;

		MessagesManager * const MsgsMgr_;
		FileTransferManager * const XferMgr_;

		QHash<QByteArray, ToxContact*> Contacts_;

		ToxAccount (const QByteArray&, const QString& name, ToxProtocol*);
	public:
		ToxAccount (const QString& name, ToxProtocol*);

		QByteArray Serialize ();
		static ToxAccount* Deserialize (const QByteArray&, ToxProtocol*);

		void SetNickname (const QString&);

		ToxContact* GetByAzothId (const QString&) const;
		ToxContact* GetByPubkey (const QByteArray&) const;

		QObject* GetQObject () override;
		QObject* GetParentProtocol () const override;
		AccountFeatures GetAccountFeatures () const override;

		QList<QObject*> GetCLEntries () override;

		QString GetAccountName () const override;
		QString GetOurNick () const override;
		void RenameAccount (const QString& name) override;
		QByteArray GetAccountID () const override;

		QList<QAction*> GetActions () const override;
		void OpenConfigurationDialog () override;
		EntryStatus GetState () const override;
		void ChangeState (const EntryStatus&) override;

		void Authorize (QObject*) override;
		void DenyAuth (QObject*) override;
		void RequestAuth (const QString&, const QString&, const QString&, const QStringList&) override;
		void RemoveEntry (QObject*) override;

		MediaCallFeatures GetMediaCallFeatures () const override;
		QObject* Call (const QString& id, const QString& variant) override;

		QObject* GetTransferManager () const override;

		void SendMessage (const QByteArray& pkey, ChatMessage *msg);
		void SetTypingState (const QByteArray& pkey, bool isTyping);
	private:
		void InitThread (const EntryStatus&);
		void InitEntry (const QByteArray&);

		void HandleConfigAccepted (AccountConfigDialog*);

		void HandleThreadReady ();

		void HandleIncomingCall (const QByteArray&, int32_t);

		void HandleToxIdRequested ();

		void HandleGotFriend (qint32);
		void HandleGotFriendRequest (const QByteArray&, const QString&);
		void HandleRemovedFriend (const QByteArray&);

		void HandleFriendNameChanged (const QByteArray&, const QString&);
		void HandleFriendStatusChanged (const QByteArray&, const EntryStatus&);
		void HandleFriendTypingChanged (const QByteArray&, bool);

		void HandleInMessage (const QByteArray&, const QString&);

		void HandleThreadFatalException (const LC::Util::QtException_ptr&);
	signals:
		void accountRenamed (const QString&) override;
		void authorizationRequested (QObject*, const QString&) override;
		void gotCLItems (const QList<QObject*>&) override;
		void itemCancelledSubscription (QObject*, const QString&) override;
		void itemGrantedSubscription (QObject*, const QString&) override;
		void itemSubscribed (QObject*, const QString&) override;
		void itemUnsubscribed (const QString& entryID, const QString&) override;
		void itemUnsubscribed (QObject*, const QString&) override;
		void mucInvitationReceived (const QVariantMap&, const QString&, const QString&) override;
		void removedCLItems (const QList< QObject* >&) override;
		void statusChanged (const EntryStatus&) override;

		void accountChanged (ToxAccount*);

		void threadChanged (const std::shared_ptr<ToxThread>&);

		void called (QObject*) override;
	};
}
}
}
