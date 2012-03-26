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

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_GLOOXACCOUNT_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_GLOOXACCOUNT_H
#include <memory>
#include <QObject>
#include <QMap>
#include <QIcon>
#include <QXmppRosterIq.h>
#include <QXmppBookmarkSet.h>
#include <interfaces/iaccount.h>
#include <interfaces/iextselfinfoaccount.h>
#include <interfaces/ihaveservicediscovery.h>
#include <interfaces/ihavesearch.h>
#include <interfaces/imessage.h>
#include <interfaces/ihaveconsole.h>
#include <interfaces/isupporttune.h>
#include <interfaces/isupportmood.h>
#include <interfaces/isupportactivity.h>
#include <interfaces/isupportgeolocation.h>
#include <interfaces/isupportmediacalls.h>
#include <interfaces/isupportriex.h>
#include <interfaces/isupportbookmarks.h>
#ifdef ENABLE_CRYPT
#include <interfaces/isupportpgp.h>
#endif
#include "glooxclentry.h"

class QXmppCall;

namespace LeechCraft
{
namespace Azoth
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

	bool operator== (const GlooxAccountState&, const GlooxAccountState&);

	class GlooxProtocol;
	class TransferManager;
	class GlooxAccountConfigurationWidget;

	class GlooxAccount : public QObject
					   , public IAccount
					   , public IExtSelfInfoAccount
					   , public IHaveServiceDiscovery
					   , public IHaveSearch
					   , public IHaveConsole
					   , public ISupportTune
					   , public ISupportMood
					   , public ISupportActivity
					   , public ISupportGeolocation
#ifdef ENABLE_MEDIACALLS
					   , public ISupportMediaCalls
#endif
					   , public ISupportRIEX
					   , public ISupportBookmarks
#ifdef ENABLE_CRYPT
					   , public ISupportPGP
#endif
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Azoth::IAccount
				LeechCraft::Azoth::IExtSelfInfoAccount
				LeechCraft::Azoth::IHaveServiceDiscovery
				LeechCraft::Azoth::IHaveSearch
				LeechCraft::Azoth::IHaveConsole
				LeechCraft::Azoth::ISupportTune
				LeechCraft::Azoth::ISupportMood
				LeechCraft::Azoth::ISupportActivity
				LeechCraft::Azoth::ISupportGeolocation
#ifdef ENABLE_MEDIACALLS
				LeechCraft::Azoth::ISupportMediaCalls
#endif
				LeechCraft::Azoth::ISupportRIEX
				LeechCraft::Azoth::ISupportBookmarks
#ifdef ENABLE_CRYPT
				LeechCraft::Azoth::ISupportPGP
#endif
			);

		QString Name_;
		GlooxProtocol *ParentProtocol_;

		QString JID_;
		QString Nick_;
		QString Resource_;
		QString Host_;
		int Port_;

		QByteArray OurPhotoHash_;

		QPair<int, int> KAParams_;

		QIcon AccountIcon_;

		std::shared_ptr<ClientConnection> ClientConnection_;
		std::shared_ptr<TransferManager> TransferManager_;

		GlooxAccountState AccState_;

		QAction *PrivacyDialogAction_;
	public:
		GlooxAccount (const QString&, QObject*);
		void Init ();

		// IAccount
		QObject* GetObject ();
		QObject* GetParentProtocol () const;
		AccountFeatures GetAccountFeatures () const;
		QList<QObject*> GetCLEntries ();
		QString GetAccountName () const;
		QString GetOurNick () const;
		QString GetHost () const;
		int GetPort () const;
		void RenameAccount (const QString&);
		QByteArray GetAccountID () const;
		QList<QAction*> GetActions () const;
		void QueryInfo (const QString&);
		void OpenConfigurationDialog ();
		void FillSettings (GlooxAccountConfigurationWidget*);
		EntryStatus GetState () const;
		void ChangeState (const EntryStatus&);
		void Authorize (QObject*);
		void DenyAuth (QObject*);
		void AddEntry (const QString&,
				const QString&, const QStringList&);
		void RequestAuth (const QString&, const QString&,
				const QString&, const QStringList&);
		void RemoveEntry (QObject*);
		QObject* GetTransferManager () const;

		// IExtSelfInfoAccount
		QObject* GetSelfContact () const;
		QImage GetSelfAvatar () const;
		QIcon GetAccountIcon () const;

		// IHaveServiceDiscovery
		QObject* CreateSDSession ();

		// IHaveSearch
		QObject* CreateSearchSession ();
		QString GetDefaultSearchServer () const;

		// IHaveConsole
		PacketFormat GetPacketFormat () const;
		void SetConsoleEnabled (bool);

		// ISupportTune, ISupportMood, ISupportActivity
		void PublishTune (const QMap<QString, QVariant>&);
		void SetMood (const QString&, const QString&);
		void SetActivity (const QString&, const QString&, const QString&);

		// ISupportGeolocation
		void SetGeolocationInfo (const GeolocationInfo_t&);
		GeolocationInfo_t GetUserGeolocationInfo (QObject*, const QString&) const;

#ifdef ENABLE_MEDIACALLS
		// ISupportMediaCalls
		MediaCallFeatures GetMediaCallFeatures () const;
		QObject* Call (const QString& id, const QString& variant);
#endif

		// ISupportRIEX
		void SuggestItems (QList<RIEXItem>, QObject*, QString);

		// ISupportBookmarks
		QWidget* GetMUCBookmarkEditorWidget ();
		QVariantList GetBookmarkedMUCs () const;
		void SetBookmarkedMUCs (const QVariantList&);

#ifdef ENABLE_CRYPT
		// ISupportPGP
		void SetPrivateKey (const QCA::PGPKey&);
		void SetEntryKey (QObject*, const QCA::PGPKey&);
		void SetEncryptionEnabled (QObject*, bool);
#endif

		QString GetJID () const;
		QString GetNick () const;
		void JoinRoom (const QString&, const QString&);
		void JoinRoom (const QString&, const QString&, const QString&);

		std::shared_ptr<ClientConnection> GetClientConnection () const;
		GlooxCLEntry* CreateFromODS (OfflineDataSource_ptr);
		QXmppBookmarkSet GetBookmarks () const;
		void SetBookmarks (const QXmppBookmarkSet&);

		void UpdateOurPhotoHash (const QByteArray&);

		void CreateSDForResource (const QString&);

		QByteArray Serialize () const;
		static GlooxAccount* Deserialize (const QByteArray&, QObject*);

		QObject* CreateMessage (IMessage::MessageType,
				const QString&, const QString&,
				const QString&);
	private:
		QString GetPassword (bool authFailure = false);
		void RegenAccountIcon ();
	public slots:
		void handleEntryRemoved (QObject*);
		void handleGotRosterItems (const QList<QObject*>&);
	private slots:
		void handleServerAuthFailed ();
		void feedClientPassword ();
		void showPrivacyDialog ();
		void handleDestroyClient ();
#ifdef ENABLE_MEDIACALLS
		void handleIncomingCall (QXmppCall*);
#endif
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

		void gotSDSession (QObject*);

		void bookmarksChanged ();

		void riexItemsSuggested (QList<LeechCraft::Azoth::RIEXItem> items,
				QObject*, QString);

		void gotConsolePacket (const QByteArray&, int);

		void geolocationInfoChanged (const QString&, QObject*);

#ifdef ENABLE_MEDIACALLS
		void called (QObject*);
#endif

#ifdef ENABLE_CRYPT
		void signatureVerified (QObject*, bool);
		void encryptionStateChanged (QObject*, bool);
#endif

		void accountSettingsChanged ();

		void scheduleClientDestruction ();
	};

	typedef std::shared_ptr<GlooxAccount> GlooxAccount_ptr;
}
}
}

#endif
