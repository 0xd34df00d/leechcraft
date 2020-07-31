/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/azoth/iaccount.h>
#include <interfaces/azoth/isupporttune.h>
#include <interfaces/azoth/iextselfinfoaccount.h>
#include <interfaces/azoth/ihaveconsole.h>
#include <interfaces/azoth/isupportnonroster.h>
#include <interfaces/azoth/ihaveserverhistory.h>
#include <interfaces/core/icoreproxy.h>
#include "structures.h"
#include "vkprotocol.h"
#include "vkconnection.h"

namespace LC
{
namespace Azoth
{
namespace Murm
{
	class VkEntry;
	class VkChatEntry;
	class VkMessage;
	class VkConnection;
	class VkConnectionTuneSetter;
	class PhotoFetcher;
	class GeoResolver;
	class GroupsManager;
	class Logger;
	class AccountConfigDialog;
	class ServerHistoryManager;
	class TransferManager;
	class AppInfoManager;

	class VkAccount : public QObject
					, public IAccount
					, public ISupportTune
					, public IExtSelfInfoAccount
					, public IHaveConsole
					, public ISupportNonRoster
					, public IHaveServerHistory
	{
		Q_OBJECT
		Q_INTERFACES (LC::Azoth::IAccount
				LC::Azoth::ISupportTune
				LC::Azoth::IExtSelfInfoAccount
				LC::Azoth::IHaveConsole
				LC::Azoth::ISupportNonRoster
				LC::Azoth::IHaveServerHistory)

		const ICoreProxy_ptr CoreProxy_;

		VkProtocol * const Proto_;
		const QByteArray ID_;

		PhotoFetcher * const PhotoStorage_;

		QString Name_;

		Logger * const Logger_;

		VkConnection * const Conn_;
		VkConnectionTuneSetter * const ConnTuneSetter_;

		GroupsManager * const GroupsMgr_;
		GeoResolver * const GeoResolver_;
		ServerHistoryManager * const ServHistMgr_;

		TransferManager * const XFerMgr_;

		AppInfoManager * const AppInfoMgr_;

		VkEntry *SelfEntry_ = nullptr;
		QHash<qulonglong, VkEntry*> Entries_;
		QHash<qulonglong, VkChatEntry*> ChatEntries_;

		QList<QPair<MessageInfo, FullMessageInfo>> PendingMessages_;

		bool PublishTune_ = false;
		bool EnableFileLog_ = false;
		bool MarkAsOnline_ = false;
		bool UpdateStatus_ = false;

		QPointer<AccountConfigDialog> AccConfigDia_;

		QList<qulonglong> NonRosterItems_;

		bool IsRequestingCaptcha_ = false;
	public:
		VkAccount (const QString& name, VkProtocol *proto, ICoreProxy_ptr proxy,
				const QByteArray& id, const QByteArray& cookies);

		QByteArray Serialize () const;
		static VkAccount* Deserialize (const QByteArray&, VkProtocol*, ICoreProxy_ptr);

		void Init ();

		void Send (qulonglong, VkConnection::Type, VkMessage*,
				const QStringList& attachments = {});
		void CreateChat (const QString&, const QList<VkEntry*>&);
		VkEntry* GetEntry (qulonglong) const;
		VkEntry* FindEntryById (const QString&) const;
		VkEntry* GetSelf () const;

		VkEntry* GetEntryOrCreate (const UserInfo&);

		ICoreProxy_ptr GetCoreProxy () const;
		VkConnection* GetConnection () const;
		PhotoFetcher* GetPhotoStorage () const;
		GeoResolver* GetGeoResolver () const;
		GroupsManager* GetGroupsManager () const;

		Logger& GetLogger () const;

		QObject* GetQObject ();
		VkProtocol* GetParentProtocol () const;
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

		void PublishTune (const QMap<QString, QVariant>& tuneData);

		QObject* GetSelfContact () const;
		QIcon GetAccountIcon () const;

		PacketFormat GetPacketFormat () const;
		void SetConsoleEnabled (bool);

		QObject* CreateNonRosterItem (const QString&);

		bool HasFeature (ServerHistoryFeature) const;
		void OpenServerHistoryConfiguration ();
		QAbstractItemModel* GetServerContactsModel () const;
		void FetchServerHistory (const QModelIndex& contact, const QByteArray& startId, int count);
		DefaultSortParams GetSortParams () const;
		QFuture<DatedFetchResult_t> FetchServerHistory (const QDateTime& since);
	private:
		void TryPendingMessages ();
		VkEntry* CreateNonRosterItem (qulonglong);

		bool CreateUsers (const QList<UserInfo>&);
	private slots:
		void handleSelfInfo (const UserInfo&);
		void handleUsers (const QList<UserInfo>&);
		void handleNRIList (const QList<qulonglong>&);
		void handleUserState (qulonglong, bool);

		void handleUserAppInfoStub (qulonglong, const AppInfo&);

		void handleMessage (const MessageInfo&);
		void handleMessage (const FullMessageInfo&, const MessageInfo&);

		void handleTypingNotification (qulonglong);

		void handleMucChanged (qulonglong);
		void handleGotChatInfo (const ChatInfo&);
		void handleChatUserRemoved (qulonglong, qulonglong);

		void handleMarkOnline ();

		void finishOffline ();

		void handleCaptcha (const QString&, const QUrl&);
		void handleCaptchaEntered (const QString&, const QString&);

		void handleConfigDialogAccepted ();

		void handleAppInfo (const AppInfo&);

		void emitUpdateAcc ();
	signals:
		void accountRenamed (const QString&);
		void gotCLItems (const QList<QObject*>&);
		void removedCLItems (const QList<QObject*>&);
		void authorizationRequested (QObject*, const QString&);
		void itemSubscribed (QObject*, const QString&);
		void itemUnsubscribed (QObject*, const QString&);
		void itemUnsubscribed (const QString&, const QString&);
		void itemCancelledSubscription (QObject*, const QString&);
		void itemGrantedSubscription (QObject*, const QString&);
		void statusChanged (const EntryStatus&);
		void mucInvitationReceived (const QVariantMap&, const QString&, const QString&);

		void accountChanged (VkAccount*);

		void gotConsolePacket (const QByteArray&, IHaveConsole::PacketDirection, const QString&);

		void serverHistoryFetched (const QModelIndex&,
				const QByteArray&, const SrvHistMessages_t&);
	};
}
}
}
