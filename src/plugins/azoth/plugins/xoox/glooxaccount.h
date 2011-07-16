/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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
#include <boost/shared_ptr.hpp>
#include <QObject>
#include <QMap>
#include <QXmppRosterIq.h>
#include <QXmppBookmarkSet.h>
#include <interfaces/iaccount.h>
#include <interfaces/ihaveservicediscovery.h>
#include <interfaces/imessage.h>
#include <interfaces/ihaveconsole.h>
#include <interfaces/isupporttune.h>
#include <interfaces/isupportmood.h>
#include <interfaces/isupportactivity.h>
#include <interfaces/isupportgeolocation.h>
#include <interfaces/isupportmediacalls.h>
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

	class GlooxProtocol;
	class TransferManager;
	class GlooxAccountConfigurationWidget;

	class GlooxAccount : public QObject
					   , public IAccount
					   , public IHaveServiceDiscovery
					   , public IHaveConsole
					   , public ISupportTune
					   , public ISupportMood
					   , public ISupportActivity
					   , public ISupportGeolocation
					   , public ISupportMediaCalls
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Azoth::IAccount
				LeechCraft::Azoth::IHaveServiceDiscovery
				LeechCraft::Azoth::IHaveConsole
				LeechCraft::Azoth::ISupportTune
				LeechCraft::Azoth::ISupportMood
				LeechCraft::Azoth::ISupportActivity
				LeechCraft::Azoth::ISupportGeolocation
				LeechCraft::Azoth::ISupportMediaCalls);

		QString Name_;
		GlooxProtocol *ParentProtocol_;

		QString JID_;
		QString Nick_;
		QString Resource_;
		QString Host_;
		int Port_;

		boost::shared_ptr<ClientConnection> ClientConnection_;
		boost::shared_ptr<TransferManager> TransferManager_;

		GlooxAccountState AccState_;
		
		QAction *PrivacyDialogAction_;
	public:
		GlooxAccount (const QString&, QObject*);
		void Init ();

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
		void Synchronize ();
		void Authorize (QObject*);
		void DenyAuth (QObject*);
		void AddEntry (const QString&,
				const QString&, const QStringList&);
		void RequestAuth (const QString&, const QString&,
				const QString&, const QStringList&);
		void RemoveEntry (QObject*);
		QObject* GetTransferManager () const;
		
		QObject* CreateSDSession ();
		
		PacketFormat GetPacketFormat () const;
		void SetConsoleEnabled (bool);
		
		void PublishTune (const QMap<QString, QVariant>&);
		void SetMood (const QString&, const QString&);
		void SetActivity (const QString&, const QString&, const QString&);
		
		void SetGeolocationInfo (const GeolocationInfo_t&);
		GeolocationInfo_t GetUserGeolocationInfo (QObject*, const QString&) const;
		
		MediaCallFeatures GetMediaCallFeatures () const;
		QObject* Call (const QString& id, const QString& variant);

		QString GetJID () const;
		QString GetNick () const;
		void JoinRoom (const QString&, const QString&, const QString&);

		boost::shared_ptr<ClientConnection> GetClientConnection () const;
		GlooxCLEntry* CreateFromODS (OfflineDataSource_ptr);
		QXmppBookmarkSet GetBookmarks () const;
		void SetBookmarks (const QXmppBookmarkSet&);

		QByteArray Serialize () const;
		static GlooxAccount* Deserialize (const QByteArray&, QObject*);

		QObject* CreateMessage (IMessage::MessageType,
				const QString&, const QString&,
				const QString&);
	private:
		QString GetPassword (bool authFailure = false);
	public slots:
		void handleEntryRemoved (QObject*);
		void handleGotRosterItems (const QList<QObject*>&);
	private slots:
		void handleServerAuthFailed ();
		void feedClientPassword ();
		void showPrivacyDialog ();
		void handleDestroyClient ();
		void handleIncomingCall (QXmppCall*);
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
		
		void gotConsolePacket (const QByteArray&, int);
		
		void geolocationInfoChanged (const QString&, QObject*);
		
		void called (QObject*);

		void accountSettingsChanged ();

		void scheduleClientDestruction ();
	};

	typedef boost::shared_ptr<GlooxAccount> GlooxAccount_ptr;
}
}
}

#endif
