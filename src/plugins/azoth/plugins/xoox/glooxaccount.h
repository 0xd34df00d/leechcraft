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
#include <QMap>
#include <QIcon>
#include <QXmppRosterIq.h>
#include <QXmppBookmarkSet.h>
#include <interfaces/azoth/iaccount.h>
#include <interfaces/azoth/iextselfinfoaccount.h>
#include <interfaces/azoth/ihaveservicediscovery.h>
#include <interfaces/azoth/ihavesearch.h>
#include <interfaces/azoth/imessage.h>
#include <interfaces/azoth/ihaveconsole.h>
#include <interfaces/azoth/isupporttune.h>
#include <interfaces/azoth/isupportmood.h>
#include <interfaces/azoth/isupportactivity.h>
#include <interfaces/azoth/isupportgeolocation.h>
#include <interfaces/azoth/isupportmediacalls.h>
#include <interfaces/azoth/isupportriex.h>
#include <interfaces/azoth/isupportbookmarks.h>
#include <interfaces/azoth/ihavemicroblogs.h>
#include <interfaces/azoth/iregmanagedaccount.h>
#include <interfaces/azoth/ihaveserverhistory.h>
#include <interfaces/azoth/isupportlastactivity.h>
#include <interfaces/azoth/ihaveblacklists.h>
#include <interfaces/azoth/icanhavesslerrors.h>
#ifdef ENABLE_CRYPT
#include <interfaces/azoth/isupportpgp.h>
#endif
#include "glooxprotocol.h"
#include "offlinedatasource.h"

namespace LC
{
namespace Azoth
{
class IProtocol;

namespace Xoox
{
	class ClientConnection;
	class AccountSettingsHolder;

	struct GlooxAccountState
	{
		State State_ = SOffline;
		QString Status_ = {};
		int Priority_ = 0;
	};

	bool operator== (const GlooxAccountState&, const GlooxAccountState&);

	class GlooxCLEntry;
	class GlooxProtocol;
	class GlooxMessage;
	class Xep0313ModelManager;

	class GlooxAccount : public QObject
					   , public IAccount
					   , public IExtSelfInfoAccount
					   , public IHaveServiceDiscovery
					   , public IHaveSearch
					   , public IHaveConsole
					   , public IHaveMicroblogs
					   , public ISupportTune
					   , public ISupportMood
					   , public ISupportActivity
					   , public ISupportGeolocation
#ifdef ENABLE_MEDIACALLS
					   , public ISupportMediaCalls
#endif
					   , public ISupportRIEX
					   , public ISupportBookmarks
					   , public ISupportLastActivity
					   , public IRegManagedAccount
					   , public IHaveServerHistory
					   , public IHaveBlacklists
					   , public ICanHaveSslErrors
#ifdef ENABLE_CRYPT
					   , public ISupportPGP
#endif
	{
		Q_OBJECT
		Q_INTERFACES (LC::Azoth::IAccount
				LC::Azoth::IExtSelfInfoAccount
				LC::Azoth::IHaveServiceDiscovery
				LC::Azoth::IHaveSearch
				LC::Azoth::IHaveConsole
				LC::Azoth::IHaveMicroblogs
				LC::Azoth::ISupportTune
				LC::Azoth::ISupportMood
				LC::Azoth::ISupportActivity
				LC::Azoth::ISupportGeolocation
				LC::Azoth::ISupportRIEX
				LC::Azoth::ISupportBookmarks
				LC::Azoth::ISupportLastActivity
				LC::Azoth::IRegManagedAccount
				LC::Azoth::IHaveServerHistory
				LC::Azoth::IHaveBlacklists
				LC::Azoth::ICanHaveSslErrors
			)

#ifdef ENABLE_MEDIACALLS
		Q_INTERFACES (LC::Azoth::ISupportMediaCalls)
#endif

#ifdef ENABLE_CRYPT
		Q_INTERFACES (LC::Azoth::ISupportPGP)
#endif

		QString Name_;
		GlooxProtocol *ParentProtocol_;

		AccountSettingsHolder *SettingsHolder_;

		QIcon AccountIcon_;

		std::shared_ptr<ClientConnection> ClientConnection_;

		struct Managers;
		std::shared_ptr<Managers> Managers_;

		QHash<QObject*, QPair<QString, QString>> ExistingEntry2JoinConflict_;

		QAction *SelfVCardAction_;
		QAction *PrivacyDialogAction_;
		QAction *CarbonsAction_;

		Xep0313ModelManager * const Xep0313ModelMgr_;
	public:
		GlooxAccount (const QString&, GlooxProtocol*, QObject*);

		void Init ();
		void Release ();

		AccountSettingsHolder* GetSettings () const;

		void AddEntry (const QString&, const QString&, const QStringList&);

		// IAccount
		QObject* GetQObject () override;
		GlooxProtocol* GetParentProtocol () const override;
		AccountFeatures GetAccountFeatures () const override;
		QList<QObject*> GetCLEntries () override;
		void SendMessage (GlooxMessage&);
		QString GetAccountName () const override;
		QString GetOurNick () const override;
		void RenameAccount (const QString&) override;
		QByteArray GetAccountID () const override;
		QList<QAction*> GetActions () const override;
		void OpenConfigurationDialog () override;
		EntryStatus GetState () const override;
		void ChangeState (const EntryStatus&) override;
		void Authorize (QObject*) override;
		void DenyAuth (QObject*) override;
		void RequestAuth (const QString&, const QString&, const QString&, const QStringList&) override;
		void RemoveEntry (QObject*) override;
		QObject* GetTransferManager () const override;

		// IExtSelfInfoAccount
		QObject* GetSelfContact () const override;
		QIcon GetAccountIcon () const override;

		// IHaveServiceDiscovery
		QObject* CreateSDSession () override;
		QString GetDefaultQuery () const override;

		// IHaveSearch
		QObject* CreateSearchSession () override;
		QString GetDefaultSearchServer () const override;

		// IHaveConsole
		PacketFormat GetPacketFormat () const override;
		void SetConsoleEnabled (bool) override;

		// IHaveMicroblogs
		void SubmitPost (const Post&) override;

		// ISupportTune, ISupportMood, ISupportActivity
		void PublishTune (const QMap<QString, QVariant>&) override;
		void SetMood (const MoodInfo&) override;
		void SetActivity (const ActivityInfo&) override;

		// ISupportGeolocation
		void SetGeolocationInfo (const GeolocationInfo_t&) override;
		GeolocationInfo_t GetUserGeolocationInfo (QObject*, const QString&) const override;

#ifdef ENABLE_MEDIACALLS
		// ISupportMediaCalls
		MediaCallFeatures GetMediaCallFeatures () const override;
		QObject* Call (const QString& id, const QString& variant) override;
#endif

		// ISupportRIEX
		void SuggestItems (QList<RIEXItem>, QObject*, QString) override;

		// ISupportBookmarks
		QWidget* GetMUCBookmarkEditorWidget () override;
		QVariantList GetBookmarkedMUCs () const override;
		void SetBookmarkedMUCs (const QVariantList&) override;

		// ISupportLastActivity
		QObject* RequestLastActivity (QObject*, const QString&) override;
		QObject* RequestLastActivity (const QString&) override;

		// IRegManagedAccount
		bool SupportsFeature (Feature) const override;
		void UpdateServerPassword (const QString& newPass) override;
		void DeregisterAccount () override;

		// IHaveServerHistory
		bool HasFeature (ServerHistoryFeature) const override;
		void OpenServerHistoryConfiguration () override;
		QAbstractItemModel* GetServerContactsModel () const override;
		void FetchServerHistory (const QModelIndex&, const QByteArray&, int) override;
		DefaultSortParams GetSortParams () const override;
		QFuture<DatedFetchResult_t> FetchServerHistory (const QDateTime&) override;

		// IHaveBlacklists
		bool SupportsBlacklists () const override;
		void SuggestToBlacklist (const QList<ICLEntry*>&) override;

#ifdef ENABLE_CRYPT
		// ISupportPGP
		void SetPrivateKey (const QCA::PGPKey&) override;
		QCA::PGPKey GetPrivateKey () const override;
		void SetEntryKey (QObject*, const QCA::PGPKey&) override;
		QCA::PGPKey GetEntryKey (QObject* entry) const override;
		GPGExceptions::MaybeException_t SetEncryptionEnabled (QObject*, bool) override;
		bool IsEncryptionEnabled (QObject*) const override;
#endif

		QString GetNick () const;
		void JoinRoom (const QString& jid, const QString& nick, const QString& password);
		void JoinRoom (const QString& room, const QString& server,
				const QString& nick, const QString& password);

		std::shared_ptr<ClientConnection> GetClientConnection () const;
		GlooxCLEntry* CreateFromODS (OfflineDataSource_ptr);

		void UpdateOurPhotoHash (const QByteArray&);

		void CreateSDForResource (const QString&);

		QByteArray Serialize () const;
		static GlooxAccount* Deserialize (const QByteArray&, GlooxProtocol*);

		GlooxMessage* CreateMessage (IMessage::Type,
				const QString&, const QString&,
				const QString&);

		QString GetDefaultReqHost () const;
	private:
		QString GetPassword (bool authFailure = false);
		void HandleClientConnectionAvailable (bool);
	public slots:
		void handleEntryRemoved (QObject*);
	signals:
		void gotCLItems (const QList<QObject*>&) override;
		void removedCLItems (const QList<QObject*>&) override;
		void accountRenamed (const QString&) override;
		void authorizationRequested (QObject*, const QString&) override;
		void itemSubscribed (QObject*, const QString&) override;
		void itemUnsubscribed (QObject*, const QString&) override;
		void itemUnsubscribed (const QString&, const QString&) override;
		void itemCancelledSubscription (QObject*, const QString&) override;
		void itemGrantedSubscription (QObject*, const QString&) override;
		void statusChanged (const EntryStatus&) override;
		void mucInvitationReceived (const QVariantMap&,
				const QString&, const QString&) override;

		void gotSDSession (QObject*) override;

		void bookmarksChanged () override;

		void riexItemsSuggested (QList<LC::Azoth::RIEXItem> items,
				QObject*, QString) override;

		void gotConsolePacket (const QByteArray&, IHaveConsole::PacketDirection, const QString&) override;

		void geolocationInfoChanged (const QString&, QObject*) override;

		void serverPasswordUpdated (const QString&) override;

		void serverHistoryFetched (const QModelIndex&,
				const QByteArray&, const SrvHistMessages_t&) override;

		void sslErrors (const QList<QSslError>&,
				const ICanHaveSslErrors::ISslErrorsReaction_ptr&) override;

#ifdef ENABLE_MEDIACALLS
		void called (QObject*) override;
#endif

#ifdef ENABLE_CRYPT
		void signatureVerified (QObject*, bool) override;
		void encryptionStateChanged (QObject*, bool) override;
#endif

		void rosterSaveRequested ();

		void accountSettingsChanged ();
	};

	typedef std::shared_ptr<GlooxAccount> GlooxAccount_ptr;
}
}
}
