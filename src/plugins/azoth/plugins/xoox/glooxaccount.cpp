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

#include "glooxaccount.h"
#include <memory>
#include <QInputDialog>
#include <QtDebug>
#include <QXmppCallManager.h>
#include <util/util.h>
#include <interfaces/azoth/iprotocol.h>
#include <interfaces/azoth/iproxyobject.h>
#include "glooxprotocol.h"
#include "glooxaccountconfigurationdialog.h"
#include "core.h"
#include "clientconnection.h"
#include "glooxmessage.h"
#include "glooxclentry.h"
#include "roomclentry.h"
#include "transfermanager.h"
#include "sdsession.h"
#include "pubsubmanager.h"
#include "usertune.h"
#include "usermood.h"
#include "useractivity.h"
#include "userlocation.h"
#include "privacylistsconfigdialog.h"

#ifdef ENABLE_MEDIACALLS
#include "mediacall.h"
#endif

#include "jabbersearchsession.h"
#include "bookmarkeditwidget.h"

#ifdef ENABLE_CRYPT
#include "pgpmanager.h"
#endif

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	bool operator== (const GlooxAccountState& s1, const GlooxAccountState& s2)
	{
		return s1.Priority_ == s2.Priority_ &&
			s1.State_ == s2.State_ &&
			s1.Status_ == s2.Status_;
	}

	GlooxAccount::GlooxAccount (const QString& name,
			QObject *parent)
	: QObject (parent)
	, Name_ (name)
	, ParentProtocol_ (qobject_cast<GlooxProtocol*> (parent))
	, Port_ (-1)
	, KAParams_ (qMakePair (90, 60))
	, PrivacyDialogAction_ (new QAction (tr ("Privacy lists..."), this))
	{
		AccState_.State_ = SOffline;
		AccState_.Priority_ = -1;

		connect (this,
				SIGNAL (scheduleClientDestruction ()),
				this,
				SLOT (handleDestroyClient ()),
				Qt::QueuedConnection);

		connect (PrivacyDialogAction_,
				SIGNAL (triggered ()),
				this,
				SLOT (showPrivacyDialog ()));
	}

	void GlooxAccount::Init ()
	{
		ClientConnection_.reset (new ClientConnection (JID_ + "/" + Resource_,
						this));

		if (!OurPhotoHash_.isEmpty ())
			ClientConnection_->SetOurPhotoHash (OurPhotoHash_);

		ClientConnection_->SetKAParams (KAParams_);

		TransferManager_.reset (new TransferManager (ClientConnection_->
						GetTransferManager (),
					this));

		connect (ClientConnection_.get (),
				SIGNAL (gotConsoleLog (QByteArray, int, QString)),
				this,
				SIGNAL (gotConsolePacket (QByteArray, int, QString)));

		connect (ClientConnection_.get (),
				SIGNAL (serverAuthFailed ()),
				this,
				SLOT (handleServerAuthFailed ()));
		connect (ClientConnection_.get (),
				SIGNAL (needPassword ()),
				this,
				SLOT (feedClientPassword ()));

		connect (ClientConnection_.get (),
				SIGNAL (statusChanged (const EntryStatus&)),
				this,
				SIGNAL (statusChanged (const EntryStatus&)));

		connect (ClientConnection_.get (),
				SIGNAL (gotRosterItems (const QList<QObject*>&)),
				this,
				SLOT (handleGotRosterItems (const QList<QObject*>&)));
		connect (ClientConnection_.get (),
				SIGNAL (rosterItemRemoved (QObject*)),
				this,
				SLOT (handleEntryRemoved (QObject*)));
		connect (ClientConnection_.get (),
				SIGNAL (rosterItemsRemoved (const QList<QObject*>&)),
				this,
				SIGNAL (removedCLItems (const QList<QObject*>&)));
		connect (ClientConnection_.get (),
				SIGNAL (gotSubscriptionRequest (QObject*, const QString&)),
				this,
				SIGNAL (authorizationRequested (QObject*, const QString&)));

		connect (ClientConnection_.get (),
				SIGNAL (rosterItemSubscribed (QObject*, const QString&)),
				this,
				SIGNAL (itemSubscribed (QObject*, const QString&)));
		connect (ClientConnection_.get (),
				SIGNAL (rosterItemUnsubscribed (QObject*, const QString&)),
				this,
				SIGNAL (itemUnsubscribed (QObject*, const QString&)));
		connect (ClientConnection_.get (),
				SIGNAL (rosterItemUnsubscribed (const QString&, const QString&)),
				this,
				SIGNAL (itemUnsubscribed (const QString&, const QString&)));
		connect (ClientConnection_.get (),
				SIGNAL (rosterItemCancelledSubscription (QObject*, const QString&)),
				this,
				SIGNAL (itemCancelledSubscription (QObject*, const QString&)));
		connect (ClientConnection_.get (),
				SIGNAL (rosterItemGrantedSubscription (QObject*, const QString&)),
				this,
				SIGNAL (itemGrantedSubscription (QObject*, const QString&)));
		connect (ClientConnection_.get (),
				SIGNAL (gotMUCInvitation (QVariantMap, QString, QString)),
				this,
				SIGNAL (mucInvitationReceived (QVariantMap, QString, QString)));
		connect (ClientConnection_.get (),
				SIGNAL (resetClientConnection ()),
				this,
				SLOT (handleResetClientConnection ()));

#ifdef ENABLE_MEDIACALLS
		connect (ClientConnection_->GetCallManager (),
				SIGNAL (callReceived (QXmppCall*)),
				this,
				SLOT (handleIncomingCall (QXmppCall*)));
#endif

		RegenAccountIcon ();
	}

	QObject* GlooxAccount::GetObject ()
	{
		return this;
	}

	QObject* GlooxAccount::GetParentProtocol () const
	{
		return ParentProtocol_;
	}

	IAccount::AccountFeatures GlooxAccount::GetAccountFeatures () const
	{
		return FRenamable | FSupportsXA | FMUCsSupportFileTransfers;
	}

	QList<QObject*> GlooxAccount::GetCLEntries ()
	{
		return ClientConnection_ ?
				ClientConnection_->GetCLEntries () :
				QList<QObject*> ();
	}

	QString GlooxAccount::GetAccountName () const
	{
		return Name_;
	}

	QString GlooxAccount::GetOurNick () const
	{
		return Nick_.isEmpty () ? JID_ : Nick_;
	}

	QString GlooxAccount::GetHost () const
	{
		return Host_;
	}

	int GlooxAccount::GetPort () const
	{
		return Port_;
	}

	void GlooxAccount::RenameAccount (const QString& name)
	{
		Name_ = name;
		emit accountRenamed (name);
		emit accountSettingsChanged ();
	}

	QByteArray GlooxAccount::GetAccountID () const
	{
		return ParentProtocol_->GetProtocolID () + "_" + JID_.toUtf8 ();
	}

	QList<QAction*> GlooxAccount::GetActions () const
	{
		QList<QAction*> result;
		result << PrivacyDialogAction_;
		return result;
	}

	void GlooxAccount::QueryInfo (const QString& entryId)
	{
		ClientConnection_->FetchVCard (entryId);
	}

	void GlooxAccount::OpenConfigurationDialog ()
	{
		std::auto_ptr<GlooxAccountConfigurationDialog> dia (new GlooxAccountConfigurationDialog (0));
		if (!JID_.isEmpty ())
			dia->W ()->SetJID (JID_);
		if (!Nick_.isEmpty ())
			dia->W ()->SetNick (Nick_);
		if (!Resource_.isEmpty ())
			dia->W ()->SetResource (Resource_);
		if (!Host_.isEmpty ())
			dia->W ()->SetHost (Host_);
		if (Port_ >= 0)
			dia->W ()->SetPort (Port_);
		dia->W ()->SetPriority (AccState_.Priority_);

		dia->W ()->SetKAInterval (KAParams_.first);
		dia->W ()->SetKATimeout (KAParams_.second);

		if (dia->exec () == QDialog::Rejected)
			return;

		FillSettings (dia->W ());
	}

	void GlooxAccount::FillSettings (GlooxAccountConfigurationWidget *w)
	{
		State lastState = AccState_.State_;
		if (lastState != SOffline &&
			(JID_ != w->GetJID () ||
			 Nick_ != w->GetNick () ||
			 Resource_ != w->GetResource () ||
			 Host_ != w->GetHost () ||
			 Port_ != w->GetPort ()))
			ChangeState (EntryStatus (SOffline, AccState_.Status_));

		if (ClientConnection_)
			ClientConnection_->SetOurJID (w->GetJID () + "/" + w->GetResource ());

		JID_ = w->GetJID ();
		Nick_ = w->GetNick ();
		Resource_ = w->GetResource ();
		AccState_.Priority_ = w->GetPriority ();
		Host_ = w->GetHost ();
		Port_ = w->GetPort ();

		RegenAccountIcon ();

		const QString& pass = w->GetPassword ();
		if (!pass.isNull ())
			Core::Instance ().GetPluginProxy ()->SetPassword (pass, this);

		KAParams_ = qMakePair (w->GetKAInterval (), w->GetKATimeout ());
		if (ClientConnection_)
			ClientConnection_->SetKAParams (KAParams_);

		if (lastState != SOffline)
			ChangeState (EntryStatus (lastState, AccState_.Status_));

		emit accountSettingsChanged ();
	}

	EntryStatus GlooxAccount::GetState () const
	{
		return EntryStatus (AccState_.State_, AccState_.Status_);
	}

	void GlooxAccount::ChangeState (const EntryStatus& status)
	{
		if (status.State_ == SOffline &&
				!ClientConnection_)
			return;

		AccState_.State_ = status.State_;
		AccState_.Status_ = status.StatusString_;

		if (!ClientConnection_)
			Init ();

		ClientConnection_->SetState (AccState_);
	}

	void GlooxAccount::Authorize (QObject *entryObj)
	{
		ClientConnection_->AckAuth (entryObj, true);
	}

	void GlooxAccount::DenyAuth (QObject *entryObj)
	{
		ClientConnection_->AckAuth (entryObj, false);
	}

	void GlooxAccount::AddEntry (const QString& entryId,
			const QString& name, const QStringList& groups)
	{
		ClientConnection_->AddEntry (entryId, name, groups);
	}

	void GlooxAccount::RequestAuth (const QString& entryId,
			const QString& msg, const QString& name, const QStringList& groups)
	{
		ClientConnection_->Subscribe (entryId, msg, name, groups);
	}

	void GlooxAccount::RemoveEntry (QObject *entryObj)
	{
		GlooxCLEntry *entry = qobject_cast<GlooxCLEntry*> (entryObj);
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< entryObj
					<< "is not a GlooxCLEntry";
			return;
		}

		ClientConnection_->Remove (entry);
	}

	QObject* GlooxAccount::GetTransferManager () const
	{
		return TransferManager_.get ();
	}

	QIcon GlooxAccount::GetAccountIcon () const
	{
		return AccountIcon_;
	}

	QObject* GlooxAccount::GetSelfContact () const
	{
		return ClientConnection_ ?
				ClientConnection_->GetCLEntry (JID_, QString ()) :
				0;
	}

	QImage GlooxAccount::GetSelfAvatar () const
	{
		auto self = GetSelfContact ();
		return self ?
				qobject_cast<ICLEntry*> (self)->GetAvatar () :
				QImage ();
	}

	QObject* GlooxAccount::CreateSDSession ()
	{
		return new SDSession (this);
	}

	QObject* GlooxAccount::CreateSearchSession ()
	{
		return new JabberSearchSession (this);
	}

	QString GlooxAccount::GetDefaultSearchServer () const
	{
		if (!Host_.isEmpty ())
			return Host_;

		const QString& second = JID_
				.split ('@', QString::SkipEmptyParts).value (1);
		const int slIdx = second.indexOf ('/');
		return slIdx >= 0 ? second.left (slIdx) : second;
	}

	IHaveConsole::PacketFormat GlooxAccount::GetPacketFormat () const
	{
		return PFXML;
	}

	void GlooxAccount::SetConsoleEnabled (bool enabled)
	{
		ClientConnection_->SetSignaledLog (enabled);
	}

	void GlooxAccount::PublishTune (const QMap<QString, QVariant>& tuneInfo)
	{
		UserTune tune;
		tune.SetArtist (tuneInfo ["artist"].toString ());
		tune.SetTitle (tuneInfo ["title"].toString ());
		tune.SetSource (tuneInfo ["source"].toString ());
		tune.SetLength (tuneInfo ["length"].toInt ());

		ClientConnection_->GetPubSubManager ()->PublishEvent (&tune);
	}

	void GlooxAccount::SetMood (const QString& moodStr, const QString& text)
	{
		UserMood mood;
		mood.SetMoodStr (moodStr);
		mood.SetText (text);

		ClientConnection_->GetPubSubManager ()->PublishEvent (&mood);
	}

	void GlooxAccount::SetActivity (const QString& general,
			const QString& specific, const QString& text)
	{
		UserActivity activity;
		activity.SetGeneralStr (general);
		activity.SetSpecificStr (specific);
		activity.SetText (text);

		ClientConnection_->GetPubSubManager ()->PublishEvent (&activity);
	}

	void GlooxAccount::SetGeolocationInfo (const GeolocationInfo_t& info)
	{
		UserLocation location;
		location.SetInfo (info);
		ClientConnection_->GetPubSubManager ()->PublishEvent (&location);
	}

	GeolocationInfo_t GlooxAccount::GetUserGeolocationInfo (QObject *obj,
			const QString& variant) const
	{
		EntryBase *entry = qobject_cast<EntryBase*> (obj);
		if (!entry)
			return GeolocationInfo_t ();

		return entry->GetGeolocationInfo (variant);
	}

#ifdef ENABLE_MEDIACALLS
	ISupportMediaCalls::MediaCallFeatures GlooxAccount::GetMediaCallFeatures () const
	{
		return MCFSupportsAudioCalls;
	}

	QObject* GlooxAccount::Call (const QString& id, const QString& variant)
	{
		if (id == qobject_cast<ICLEntry*> (GetSelfContact ())->GetEntryID ())
		{
			Core::Instance ().SendEntity (Util::MakeNotification ("LeechCraft",
						tr ("Why would you call yourself?"),
						PWarning_));

			return 0;
		}

		QString target = GlooxCLEntry::JIDFromID (this, id);

		QString var = variant;
		if (var.isEmpty ())
		{
			QObject *entryObj = GetClientConnection ()->
					GetCLEntry (target, QString ());
			GlooxCLEntry *entry = qobject_cast<GlooxCLEntry*> (entryObj);
			if (entry)
				var = entry->Variants ().value (0);
			else
				qWarning () << Q_FUNC_INFO
						<< "null entry for"
						<< target;
		}
		target += '/' + var;
		return new MediaCall (this, ClientConnection_->GetCallManager ()->call (target));
	}
#endif

	void GlooxAccount::SuggestItems (QList<RIEXItem> items, QObject *to, QString message)
	{
		EntryBase *entry = qobject_cast<EntryBase*> (to);
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to cast"
					<< to
					<< "to EntryBase";
			return;
		}

		QList<RIEXManager::Item> add;
		QList<RIEXManager::Item> del;
		QList<RIEXManager::Item> modify;
		Q_FOREACH (const RIEXItem& item, items)
		{
			switch (item.Action_)
			{
			case RIEXItem::AAdd:
				add << RIEXManager::Item (RIEXManager::Item::AAdd,
						item.ID_, item.Nick_, item.Groups_);
				break;
			case RIEXItem::ADelete:
				del << RIEXManager::Item (RIEXManager::Item::ADelete,
						item.ID_, item.Nick_, item.Groups_);
				break;
			case RIEXItem::AModify:
				modify << RIEXManager::Item (RIEXManager::Item::AModify,
						item.ID_, item.Nick_, item.Groups_);
				break;
			default:
				qWarning () << Q_FUNC_INFO
						<< "unknown action"
						<< item.Action_
						<< "for item"
						<< item.ID_;
				break;
			}
		}

		if (!add.isEmpty ())
			ClientConnection_->GetRIEXManager ()->SuggestItems (entry, add, message);
		if (!modify.isEmpty ())
			ClientConnection_->GetRIEXManager ()->SuggestItems (entry, modify, message);
		if (!del.isEmpty ())
			ClientConnection_->GetRIEXManager ()->SuggestItems (entry, del, message);
	}

	QWidget* GlooxAccount::GetMUCBookmarkEditorWidget ()
	{
		return new BookmarkEditWidget ();
	}

	QVariantList GlooxAccount::GetBookmarkedMUCs () const
	{
		QVariantList result;

		const QXmppBookmarkSet& set = GetBookmarks ();

		Q_FOREACH (const QXmppBookmarkConference& conf, set.conferences ())
		{
			const QStringList& split = conf.jid ().split ('@', QString::SkipEmptyParts);
			if (split.size () != 2)
			{
				qWarning () << Q_FUNC_INFO
						<< "incorrectly split jid for conf"
						<< conf.jid ()
						<< split;
				continue;
			}

			QVariantMap cm;
			cm ["HumanReadableName"] = QString ("%1 (%2)")
					.arg (conf.jid ())
					.arg (conf.nickName ());
			cm ["AccountID"] = GetAccountID ();
			cm ["Nick"] = conf.nickName ();
			cm ["Room"] = split.at (0);
			cm ["Server"] = split.at (1);
			cm ["Autojoin"] = conf.autoJoin ();
			cm ["StoredName"] = conf.name ();
			result << cm;
		}

		return result;
	}

	void GlooxAccount::SetBookmarkedMUCs (const QVariantList& datas)
	{
		QList<QXmppBookmarkConference> mucs;
		Q_FOREACH (const QVariant& var, datas)
		{
			const QVariantMap& map = var.toMap ();
			QXmppBookmarkConference conf;
			conf.setAutoJoin (map.value ("Autojoin").toBool ());
			conf.setJid (map.value ("Room").toString () + '@' + map.value ("Server").toString ());
			conf.setNickName (map.value ("Nick").toString ());
			conf.setName (map.value ("StoredName").toString ());
			mucs << conf;
		}

		QXmppBookmarkSet set;
		set.setConferences (mucs);
		set.setUrls (GetBookmarks ().urls ());
		SetBookmarks (set);
	}

#ifdef ENABLE_CRYPT
	void GlooxAccount::SetPrivateKey (const QCA::PGPKey& key)
	{
		ClientConnection_->GetPGPManager ()->SetPrivateKey (key);
	}

	void GlooxAccount::SetEntryKey (QObject *entryObj, const QCA::PGPKey& pubKey)
	{
		ICLEntry *entry = qobject_cast<ICLEntry*> (entryObj);
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< entryObj
					<< "doesn't implement ICLEntry";
			return;
		}

		ClientConnection_->GetPGPManager ()->SetPublicKey (entry->GetHumanReadableID (), pubKey);
	}

	void GlooxAccount::SetEncryptionEnabled (QObject *entry, bool enabled)
	{
		GlooxCLEntry *glEntry = qobject_cast<GlooxCLEntry*> (entry);
		if (!glEntry)
			return;

		const QString& jid = glEntry->GetJID ();
		if (enabled &&
				ClientConnection_->GetPGPManager ()->PublicKey (jid).isNull ())
		{
			Core::Instance ().SendEntity (Util::MakeNotification ("Azoth",
						tr ("Unable to enable encryption for entry %1: "
							"no key has been set.")
								.arg (glEntry->GetEntryName ()),
						PCritical_));
			return;
		}

		if (!ClientConnection_->SetEncryptionEnabled (jid, enabled))
			Core::Instance ().SendEntity (Util::MakeNotification ("Azoth",
						tr ("Unable to change encryption state for %1.")
								.arg (glEntry->GetEntryName ()),
						PCritical_));
		else
			emit encryptionStateChanged (entry, enabled);
	}
#endif

	QString GlooxAccount::GetJID () const
	{
		return JID_;
	}

	QString GlooxAccount::GetNick () const
	{
		return Nick_.isEmpty () ? JID_ : Nick_;
	}

	void GlooxAccount::JoinRoom (const QString& jid, const QString& nick)
	{
		if (!ClientConnection_)
		{
			qWarning () << Q_FUNC_INFO
					<< "null ClientConnection";
			return;
		}

		RoomCLEntry *entry = ClientConnection_->JoinRoom (jid, nick);
		if (!entry)
			return;
		emit gotCLItems (QList<QObject*> () << entry);
	}

	void GlooxAccount::JoinRoom (const QString& server,
			const QString& room, const QString& nick)
	{
		QString jidStr = QString ("%1@%2")
				.arg (room, server);
		JoinRoom (jidStr, nick);
	}

	std::shared_ptr<ClientConnection> GlooxAccount::GetClientConnection () const
	{
		return ClientConnection_;
	}

	GlooxCLEntry* GlooxAccount::CreateFromODS (OfflineDataSource_ptr ods)
	{
		return ClientConnection_->AddODSCLEntry (ods);
	}

	QXmppBookmarkSet GlooxAccount::GetBookmarks () const
	{
		if (!ClientConnection_)
			return QXmppBookmarkSet ();

		return ClientConnection_->GetBookmarks ();
	}

	void GlooxAccount::SetBookmarks (const QXmppBookmarkSet& set)
	{
		if (!ClientConnection_)
			return;

		ClientConnection_->SetBookmarks (set);
	}

	void GlooxAccount::UpdateOurPhotoHash (const QByteArray& hash)
	{
		if (hash == OurPhotoHash_)
			return;

		OurPhotoHash_ = hash;
		ClientConnection_->SetOurPhotoHash (hash);
		ChangeState (GetState ());

		emit accountSettingsChanged ();
	}

	void GlooxAccount::CreateSDForResource (const QString& resource)
	{
		auto sd = new SDSession (this);
		sd->SetQuery (resource);
		emit gotSDSession (sd);
	}

	QByteArray GlooxAccount::Serialize () const
	{
		quint16 version = 4;

		QByteArray result;
		{
			QDataStream ostr (&result, QIODevice::WriteOnly);
			ostr << version
				<< Name_
				<< JID_
				<< Nick_
				<< Resource_
				<< AccState_.Priority_
				<< Host_
				<< Port_
				<< KAParams_
				<< OurPhotoHash_;
		}

		return result;
	}

	GlooxAccount* GlooxAccount::Deserialize (const QByteArray& data, QObject *parent)
	{
		quint16 version = 0;

		QDataStream in (data);
		in >> version;

		if (version < 1 || version > 4)
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown version"
					<< version;
			return 0;
		}

		QString name;
		in >> name;
		GlooxAccount *result = new GlooxAccount (name, parent);
		in >> result->JID_
			>> result->Nick_
			>> result->Resource_
			>> result->AccState_.Priority_;
		if (version >= 2)
			in >> result->Host_
				>> result->Port_;
		if (version >= 3)
			in >> result->KAParams_;
		if (version >= 4)
			in >> result->OurPhotoHash_;
		result->Init ();

		return result;
	}

	QObject* GlooxAccount::CreateMessage (IMessage::MessageType type,
			const QString& variant,
			const QString& body,
			const QString& jid)
	{
		return ClientConnection_->CreateMessage (type, variant, body, jid);
	}

	QString GlooxAccount::GetPassword (bool authfailure)
	{
		IProxyObject *proxy =
			qobject_cast<IProxyObject*> (ParentProtocol_->GetProxyObject ());
		return proxy->GetAccountPassword (this, !authfailure);
	}

	void GlooxAccount::RegenAccountIcon ()
	{
		AccountIcon_ = QIcon ();

		if (JID_.contains ("google") ||
				JID_.contains ("gmail"))
			AccountIcon_ = QIcon (":/plugins/azoth/plugins/xoox/resources/images/special/gtalk.svg");
		else if (JID_.contains ("facebook") ||
				JID_.contains ("fb.com"))
			AccountIcon_ = QIcon (":/plugins/azoth/plugins/xoox/resources/images/special/facebook.svg");
		else if (JID_.contains ("vk.com"))
			AccountIcon_ = QIcon (":/plugins/azoth/plugins/xoox/resources/images/special/vk.svg");
	}

	void GlooxAccount::handleEntryRemoved (QObject *entry)
	{
		emit removedCLItems (QObjectList () << entry);
	}

	void GlooxAccount::handleGotRosterItems (const QList<QObject*>& items)
	{
		emit gotCLItems (items);
	}

	void GlooxAccount::handleServerAuthFailed ()
	{
		const QString& pwd = GetPassword (true);
		if (!pwd.isNull ())
		{
			ClientConnection_->SetPassword (pwd);
			ChangeState (EntryStatus (AccState_.State_, AccState_.Status_));
		}
	}

	void GlooxAccount::feedClientPassword ()
	{
		ClientConnection_->SetPassword (GetPassword ());
	}

	void GlooxAccount::showPrivacyDialog ()
	{
		PrivacyListsManager *mgr = ClientConnection_->GetPrivacyListsManager ();
		PrivacyListsConfigDialog *plcd = new PrivacyListsConfigDialog (mgr);
		plcd->show ();
	}

	void GlooxAccount::handleResetClientConnection ()
	{
		TransferManager_.reset ();
		Init ();
		ChangeState (GetState ());
	}

	void GlooxAccount::handleDestroyClient ()
	{
		ClientConnection_.reset ();
	}

#ifdef ENABLE_MEDIACALLS
	void GlooxAccount::handleIncomingCall (QXmppCall *call)
	{
		emit called (new MediaCall (this, call));
	}
#endif
}
}
}
