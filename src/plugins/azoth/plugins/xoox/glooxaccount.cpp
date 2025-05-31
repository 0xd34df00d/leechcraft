/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "glooxaccount.h"
#include <memory>
#include <QInputDialog>
#include <QMessageBox>
#include <QtDebug>
#include <QAction>
#include <QXmppMucManager.h>
#include <util/xpc/util.h>
#include <util/sll/prelude.h>
#include <util/sll/slotclosure.h>
#include <util/sll/util.h>
#include <util/sll/either.h>
#include <util/sll/void.h>
#include <util/threads/futures.h>
#include <interfaces/azoth/iprotocol.h>
#include <interfaces/azoth/iproxyobject.h>

#ifdef ENABLE_CRYPT
#include "xeps/pgpmanager.h"
#endif

#include "glooxprotocol.h"
#include "core.h"
#include "clientconnection.h"
#include "glooxmessage.h"
#include "glooxclentry.h"
#include "roomclentry.h"
#include "transfermanager.h"
#include "sdsession.h"
#include "xeps/pubsubmanager.h"
#include "usertune.h"
#include "usermood.h"
#include "useractivity.h"
#include "userlocation.h"
#include "privacylistsconfigdialog.h"
#include "pepmicroblog.h"
#include "jabbersearchsession.h"
#include "bookmarkeditwidget.h"
#include "accountsettingsholder.h"
#include "crypthandler.h"
#include "gwitemsremovaldialog.h"
#include "serverinfostorage.h"
#include "xeps/xep0313manager.h"
#include "xep0313prefsdialog.h"
#include "xep0313modelmanager.h"
#include "pendinglastactivityrequest.h"
#include "xeps/lastactivitymanager.h"
#include "roomhandler.h"
#include "clientconnectionerrormgr.h"
#include "clientconnectionextensionsmanager.h"
#include "addtoblockedrunner.h"
#include "util.h"
#include "selfcontact.h"
#include "callshandler.h"
#include "bookmarksintegrator.h"
#include "clientloggermanager.h"
#include "xeps/riexmanager.h"
#include "riexintegrator.h"
#include "inbandaccountactions.h"
#include "captchamanager.h"
#include "deliveryreceiptsintegrator.h"

namespace LC
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

	namespace
	{
		QIcon MakeAccountIcon (const QString& jid)
		{
			if (jid.contains ("google") ||
					jid.contains ("gmail"))
				return QIcon { ":/plugins/azoth/plugins/xoox/resources/images/special/gtalk.svg" };
			else if (jid.contains ("facebook") ||
					jid.contains ("fb.com"))
				return QIcon { ":/plugins/azoth/plugins/xoox/resources/images/special/facebook.svg" };
			else if (jid.contains ("odnoklassniki"))
				return QIcon { ":/plugins/azoth/plugins/xoox/resources/images/special/odnoklassniki.svg" };

			return {};
		}
	}

	GlooxAccount::GlooxAccount (const QString& name,
			GlooxProtocol *proto,
			QObject *parent)
	: QObject (parent)
	, Name_ (name)
	, ParentProtocol_ (proto)
	, SettingsHolder_ (new AccountSettingsHolder (this))
	, SelfVCardAction_ (new QAction (tr ("Self VCard..."), this))
	, PrivacyDialogAction_ (new QAction (tr ("Privacy lists..."), this))
	, CarbonsAction_ (new QAction (tr ("Enable message carbons"), this))
	, Xep0313ModelMgr_ (new Xep0313ModelManager (this))
	{
		SelfVCardAction_->setProperty ("ActionIcon", "text-x-vcard");
		PrivacyDialogAction_->setProperty ("ActionIcon", "emblem-locked");

		CarbonsAction_->setProperty ("ActionIcon", "edit-copy");
		CarbonsAction_->setCheckable (true);
		CarbonsAction_->setToolTip (tr ("Deliver messages from conversations on "
					"other resources to this resource as well."));

		connect (SelfVCardAction_,
				&QAction::triggered,
				[this]
				{
					const auto& jid = SettingsHolder_->GetJID ();
					if (auto entry = qobject_cast<EntryBase*> (ClientConnection_->GetCLEntry (jid)))
						entry->ShowInfo ();
				});
		connect (PrivacyDialogAction_,
				&QAction::triggered,
				[this] { (new PrivacyListsConfigDialog (ClientConnection_->GetPrivacyListsManager ()))->show (); });
		connect (CarbonsAction_,
				&QAction::toggled,
				[this] (bool enabled) { SettingsHolder_->SetMessageCarbonsEnabled (enabled); });

		connect (SettingsHolder_,
				&AccountSettingsHolder::accountSettingsChanged,
				this,
				&GlooxAccount::accountSettingsChanged);
		connect (SettingsHolder_,
				&AccountSettingsHolder::jidChanged,
				[this] (const QString& jid) { AccountIcon_ = MakeAccountIcon (jid); });

		HandleClientConnectionAvailable (false);
	}

	struct GlooxAccount::Managers
	{
		ClientConnection& Conn_;
		GlooxAccount& Acc_;

		ClientConnectionExtensionsManager& ExtsMgr_ = Conn_.Exts ();

		TransferManager TransferManager_ { ExtsMgr_.Get<QXmppTransferManager> (), Conn_, Acc_ };
		BookmarksIntegrator BookmarksIntegrator_ { ExtsMgr_.Get<QXmppBookmarkManager> (), Conn_, Acc_ };
		RIEXIntegrator RiexIntegrator_ { ExtsMgr_.Get<RIEXManager> (), Acc_ };
#ifdef ENABLE_MEDIACALLS
		CallsHandler CallsHandler_ { ExtsMgr_.Get<QXmppCallManager> (), Conn_, Acc_ };
#endif
		ClientLoggerManager ClientLoggerManager_ { *Conn_.GetClient (), *Acc_.GetSettings () };
		InBandAccountActions AccountActions_ { Conn_, Acc_ };

		CaptchaManager CaptchaManager_ { ExtsMgr_.Get<XMPPCaptchaManager> (), ExtsMgr_.Get<XMPPBobManager> () };

		DeliveryReceiptsIntegrator ReceiptsIntegrator_ { ExtsMgr_.Get<QXmppMessageReceiptManager> () };

		Managers (ClientConnection& conn, GlooxAccount& acc)
		: Conn_ { conn }
		, Acc_ { acc }
		{
		}
	};

	void GlooxAccount::Init ()
	{
		ClientConnection_ = std::make_shared<ClientConnection> (this);

		connect (ClientConnection_.get (),
				&ClientConnection::sslErrors,
				this,
				&GlooxAccount::sslErrors);

		Managers_ = std::make_shared<Managers> (*ClientConnection_, *this);

		connect (&Managers_->ClientLoggerManager_,
				&ClientLoggerManager::gotConsoleLog,
				this,
				&GlooxAccount::gotConsolePacket);

		connect (ClientConnection_.get (),
				&ClientConnection::serverAuthFailed,
				[this]
				{
					const auto& pwd = GetPassword (true);
					if (!pwd.isNull ())
					{
						ClientConnection_->SetPassword (pwd);
						ClientConnection_->SetState (ClientConnection_->GetLastState ());
					}
				});
		connect (ClientConnection_.get (),
				&ClientConnection::needPassword,
				[this] { ClientConnection_->SetPassword (GetPassword ()); });

		connect (ClientConnection_.get (),
				&ClientConnection::statusChanged,
				this,
				&GlooxAccount::statusChanged);

		connect (ClientConnection_.get (),
				&ClientConnection::gotRosterItems,
				this,
				&GlooxAccount::gotCLItems);
		connect (ClientConnection_.get (),
				SIGNAL (rosterItemRemoved (QObject*)),
				this,
				SLOT (handleEntryRemoved (QObject*)));

		connect (ClientConnection_->GetXep0313Manager (),
				&Xep0313Manager::serverHistoryFetched,
				this,
				[this] (const QString& jid, const QString& id, const SrvHistMessages_t& messages)
				{
					emit serverHistoryFetched (Xep0313ModelMgr_->Jid2Index (jid), id.toUtf8 (), messages);
				});

		AccountIcon_ = MakeAccountIcon (SettingsHolder_->GetJID ());

		CarbonsAction_->setChecked (SettingsHolder_->IsMessageCarbonsEnabled ());

		HandleClientConnectionAvailable (true);
	}

	void GlooxAccount::Release ()
	{
		emit removedCLItems (GetCLEntries ());
	}

	AccountSettingsHolder* GlooxAccount::GetSettings () const
	{
		return SettingsHolder_;
	}

	QObject* GlooxAccount::GetQObject ()
	{
		return this;
	}

	GlooxProtocol* GlooxAccount::GetParentProtocol () const
	{
		return ParentProtocol_;
	}

	IAccount::AccountFeatures GlooxAccount::GetAccountFeatures () const
	{
		return FRenamable | FSupportsXA | FMUCsSupportFileTransfers | FCanViewContactsInfoInOffline;
	}

	QList<QObject*> GlooxAccount::GetCLEntries ()
	{
		return ClientConnection_ ?
				ClientConnection_->GetCLEntries () :
				QList<QObject*> ();
	}

	void GlooxAccount::SendMessage (GlooxMessage& msg)
	{
		Managers_->ReceiptsIntegrator_.ProcessMessage (msg);
		ClientConnection_->SendMessage (&msg);
	}

	QString GlooxAccount::GetAccountName () const
	{
		return Name_;
	}

	QString GlooxAccount::GetOurNick () const
	{
		return SettingsHolder_->GetNick ();
	}

	void GlooxAccount::RenameAccount (const QString& name)
	{
		Name_ = name;
		emit accountRenamed (name);
		emit accountSettingsChanged ();
	}

	QByteArray GlooxAccount::GetAccountID () const
	{
		return ParentProtocol_->GetProtocolID () + "_" + SettingsHolder_->GetJID ().toUtf8 ();
	}

	QList<QAction*> GlooxAccount::GetActions () const
	{
		return { SelfVCardAction_, PrivacyDialogAction_, CarbonsAction_ };
	}

	void GlooxAccount::OpenConfigurationDialog ()
	{
		SettingsHolder_->OpenConfigDialog ();
	}

	EntryStatus GlooxAccount::GetState () const
	{
		const auto& state = ClientConnection_ ?
				ClientConnection_->GetLastState () :
				GlooxAccountState ();
		return EntryStatus (state.State_, state.Status_);
	}

	void GlooxAccount::ChangeState (const EntryStatus& status)
	{
		if (status.State_ == SOffline &&
				!ClientConnection_)
			return;

		if (!ClientConnection_)
			Init ();

		auto state = ClientConnection_->GetLastState ();
		state.State_ = status.State_;
		state.Status_ = status.StatusString_;
		ClientConnection_->SetState (state);
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
		auto entry = qobject_cast<GlooxCLEntry*> (entryObj);
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< entryObj
					<< "is not a GlooxCLEntry";
			return;
		}

		if (entry->IsGateway ())
		{
			const auto& allEntries = ClientConnection_->GetCLEntries ();

			const auto& gwJid = entry->GetJID ();

			QList<GlooxCLEntry*> subs;
			for (auto obj : allEntries)
			{
				auto otherEntry = qobject_cast<GlooxCLEntry*> (obj);
				if (otherEntry &&
						otherEntry != entry &&
						otherEntry->GetJID ().endsWith (gwJid))
					subs << otherEntry;
			}

			if (!subs.isEmpty ())
			{
				GWItemsRemovalDialog dia (subs);
				if (dia.exec () == QDialog::Accepted)
					for (auto subEntry : subs)
						RemoveEntry (subEntry);
			}
		}

		ClientConnection_->Remove (entry);
	}

	QObject* GlooxAccount::GetTransferManager () const
	{
		return &Managers_->TransferManager_;
	}

	QIcon GlooxAccount::GetAccountIcon () const
	{
		return AccountIcon_;
	}

	QObject* GlooxAccount::GetSelfContact () const
	{
		return ClientConnection_ ?
				ClientConnection_->GetCLEntry (SettingsHolder_->GetJID (), QString ()) :
				0;
	}

	QObject* GlooxAccount::CreateSDSession ()
	{
		return new SDSession (this);
	}

	QString GlooxAccount::GetDefaultQuery () const
	{
		return GetDefaultReqHost ();
	}

	QObject* GlooxAccount::CreateSearchSession ()
	{
		return new JabberSearchSession (this);
	}

	QString GlooxAccount::GetDefaultSearchServer () const
	{
		return GetDefaultReqHost ();
	}

	IHaveConsole::PacketFormat GlooxAccount::GetPacketFormat () const
	{
		return PacketFormat::XML;
	}

	void GlooxAccount::SetConsoleEnabled (bool enabled)
	{
		Managers_->ClientLoggerManager_.SetSignaledLog (enabled);
	}

	void GlooxAccount::SubmitPost (const Post& post)
	{
		PEPMicroblog micro (post);
		ClientConnection_->GetPubSubManager ()->PublishEvent (&micro);
	}

	void GlooxAccount::PublishTune (const QMap<QString, QVariant>& tuneInfo)
	{
		UserTune tune;
		tune.SetArtist (tuneInfo ["artist"].toString ());
		tune.SetTitle (tuneInfo ["title"].toString ());
		tune.SetSource (tuneInfo ["source"].toString ());
		tune.SetLength (tuneInfo ["length"].toInt ());

		if (tuneInfo.contains ("track"))
		{
			const int track = tuneInfo ["track"].toInt ();
			if (track > 0)
				tune.SetTrack (QString::number (track));
		}

		ClientConnection_->GetPubSubManager ()->PublishEvent (&tune);
	}

	void GlooxAccount::SetMood (const MoodInfo& moodInfo)
	{
		UserMood mood;
		mood.SetMoodStr (moodInfo.Mood_);
		mood.SetText (moodInfo.Text_);

		ClientConnection_->GetPubSubManager ()->PublishEvent (&mood);
	}

	void GlooxAccount::SetActivity (const ActivityInfo& info)
	{
		UserActivity activity;
		activity.SetGeneralStr (info.General_);
		activity.SetSpecificStr (info.Specific_);
		activity.SetText (info.Text_);

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
		const auto entry = qobject_cast<EntryBase*> (obj);
		return entry ? entry->GetGeolocationInfo (variant) : GeolocationInfo_t {};
	}

#ifdef ENABLE_MEDIACALLS
	ISupportMediaCalls::MediaCallFeatures GlooxAccount::GetMediaCallFeatures () const
	{
		return MCFSupportsAudioCalls;
	}

	QObject* GlooxAccount::Call (const QString& id, const QString& variant)
	{
		return Managers_->CallsHandler_.Call (id, variant);
	}
#endif

	void GlooxAccount::SuggestItems (QList<RIEXItem> items, QObject *to, QString message)
	{
		Managers_->RiexIntegrator_.SuggestItems (items, to, message);
	}

	QWidget* GlooxAccount::GetMUCBookmarkEditorWidget ()
	{
		return new BookmarkEditWidget ();
	}

	QVariantList GlooxAccount::GetBookmarkedMUCs () const
	{
		return Managers_->BookmarksIntegrator_.GetBookmarkedMUCs ();
	}

	void GlooxAccount::SetBookmarkedMUCs (const QVariantList& datas)
	{
		Managers_->BookmarksIntegrator_.SetBookmarkedMUCs (datas);
	}

	QObject* GlooxAccount::RequestLastActivity (QObject *entry, const QString& variant)
	{
		auto jid = qobject_cast<ICLEntry*> (entry)->GetHumanReadableID ();
		if (!variant.isEmpty ())
			jid += '/' + variant;
		return RequestLastActivity (jid);
	}

	QObject* GlooxAccount::RequestLastActivity (const QString& jid)
	{
		auto pending = new PendingLastActivityRequest { jid, this };

		auto& manager = ClientConnection_->Exts ().Get<LastActivityManager> ();
		const auto& id = manager.RequestLastActivity (jid);
		connect (&manager,
				SIGNAL (gotLastActivity (QString, int)),
				pending,
				SLOT (handleGotLastActivity (QString, int)));

		ClientConnection_->GetErrorManager ()->Whitelist (id);
		ClientConnection_->AddCallback (id,
				[pending] (const QXmppIq& iq)
				{
					if (iq.type () == QXmppIq::Error)
						pending->deleteLater ();
				});

		return pending;
	}

	bool GlooxAccount::SupportsFeature (Feature f) const
	{
		switch (f)
		{
		case Feature::UpdatePass:
		case Feature::DeregisterAcc:
			return true;
		}

		return false;
	}

	void GlooxAccount::UpdateServerPassword (const QString& newPass)
	{
		Managers_->AccountActions_.UpdateServerPassword (newPass);
	}

	void GlooxAccount::DeregisterAccount ()
	{
		Managers_->AccountActions_.CancelRegistration ();
	}

	bool GlooxAccount::HasFeature (ServerHistoryFeature feature) const
	{
		auto infoStorage = ClientConnection_->GetServerInfoStorage ();
		const bool supportsMam = Xep0313Manager::Supports0313 (infoStorage->GetServerFeatures ()) ||
				Xep0313Manager::Supports0313 (infoStorage->GetSelfFeatures ());
		switch (feature)
		{
		case ServerHistoryFeature::AccountSupportsHistory:
		case ServerHistoryFeature::Configurable:
			return supportsMam;
		case ServerHistoryFeature::DatedFetching:
			// TODO
			return false;
		}

		qWarning () << Q_FUNC_INFO
				<< "unknown feature"
				<< static_cast<int> (feature);
		return false;
	}

	void GlooxAccount::OpenServerHistoryConfiguration ()
	{
		auto dialog = new Xep0313PrefsDialog (ClientConnection_->GetXep0313Manager ());
		dialog->show ();
	}

	QAbstractItemModel* GlooxAccount::GetServerContactsModel () const
	{
		return Xep0313ModelMgr_->GetModel ();
	}

	void GlooxAccount::FetchServerHistory (const QModelIndex& index,
			const QByteArray& startId, int count)
	{
		const auto& jid = Xep0313ModelMgr_->Index2Jid (index);
		ClientConnection_->GetXep0313Manager ()->RequestHistory (jid, startId, count);
	}

	DefaultSortParams GlooxAccount::GetSortParams () const
	{
		return { 0, Qt::DisplayRole, Qt::AscendingOrder };
	}

	QFuture<IHaveServerHistory::DatedFetchResult_t> GlooxAccount::FetchServerHistory (const QDateTime&)
	{
		return Util::MakeReadyFuture (IHaveServerHistory::DatedFetchResult_t::Left ("Not implemented yet."));
	}

	bool GlooxAccount::SupportsBlacklists () const
	{
		if (!ClientConnection_)
			return false;

		return ClientConnection_->GetPrivacyListsManager ()->IsSupported ();
	}

	void GlooxAccount::SuggestToBlacklist (const QList<ICLEntry*>& entries)
	{
		if (!ClientConnection_)
		{
			qWarning () << Q_FUNC_INFO
					<< "no client connection is instantiated";
			return;
		}

		bool ok = false;
		const QStringList variants { tr ("By full JID"), tr ("By domain") };
		const auto& selected = QInputDialog::getItem (nullptr,
				"LeechCraft",
				tr ("Select block type:"),
				variants,
				0,
				false,
				&ok);
		if (!ok)
			return;

		QStringList allJids { Util::Map (entries, [] (ICLEntry *entry) { return entry->GetHumanReadableID (); }) };
		if (variants.indexOf (selected) == 1)
			allJids = Util::Map (allJids,
					[] (const QString& jid) { return ClientConnection::Split (jid).Bare_.section ('@', 1); });

		allJids.removeDuplicates ();

		new AddToBlockedRunner { allJids, ClientConnection_, this };
	}

#ifdef ENABLE_CRYPT
	void GlooxAccount::SetPrivateKey (const QCA::PGPKey& key)
	{
		ClientConnection_->GetCryptHandler ()->GetPGPManager ()->SetPrivateKey (key);
	}

	QCA::PGPKey GlooxAccount::GetPrivateKey () const
	{
		return ClientConnection_->GetCryptHandler ()->GetPGPManager ()->PrivateKey ();
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

		auto mgr = ClientConnection_->GetCryptHandler ()->GetPGPManager ();
		mgr->SetPublicKey (entry->GetHumanReadableID (), pubKey);
	}

	QCA::PGPKey GlooxAccount::GetEntryKey (QObject *entryObj) const
	{
		ICLEntry *entry = qobject_cast<ICLEntry*> (entryObj);
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< entryObj
					<< "doesn't implement ICLEntry";
			return QCA::PGPKey ();
		}

		auto mgr = ClientConnection_->GetCryptHandler ()->GetPGPManager ();
		return mgr->PublicKey (entry->GetHumanReadableID ());
	}

	GPGExceptions::MaybeException_t GlooxAccount::SetEncryptionEnabled (QObject *entry, bool enabled)
	{
		const auto glEntry = qobject_cast<GlooxCLEntry*> (entry);

		const auto cryptHandler = ClientConnection_->GetCryptHandler ();
		const auto pgpManager = cryptHandler->GetPGPManager ();

		bool beenChanged = false;
		const auto emitGuard = Util::MakeScopeGuard ([&]
					{ emit encryptionStateChanged (entry, beenChanged ? enabled : !enabled); });

		if (!glEntry)
			return GPGExceptions::General { "Null entry" };
		if (enabled && pgpManager->PublicKey (glEntry->GetJID ()).isNull ())
			return GPGExceptions::NullPubkey {};
		if (!cryptHandler->SetEncryptionEnabled (glEntry->GetJID (), enabled))
			return GPGExceptions::General { "Cannot change encryption state. "};
		beenChanged = true;
		return {};
	}

	bool GlooxAccount::IsEncryptionEnabled (QObject *entry) const
	{
		const auto glEntry = qobject_cast<GlooxCLEntry*> (entry);
		if (!glEntry)
			return false;

		return ClientConnection_->GetCryptHandler ()->IsEncryptionEnabled (glEntry->GetJID ());
	}
#endif

	QString GlooxAccount::GetNick () const
	{
		return SettingsHolder_->GetNick ();
	}

	namespace
	{
		QString NormalizeRoomJid (const QString& jid)
		{
			return jid.toLower ();
		}
	}

	void GlooxAccount::JoinRoom (const QString& origJid, const QString& nick, const QString& password)
	{
		if (!ClientConnection_)
		{
			qWarning () << Q_FUNC_INFO
					<< "null ClientConnection";
			return;
		}

		auto jid = NormalizeRoomJid (origJid);
		if (jid != origJid)
			qWarning () << Q_FUNC_INFO
					<< "room jid normalization happened from"
					<< origJid
					<< "to"
					<< jid;

		const auto existingObj = ClientConnection_->GetCLEntry (jid, {});
		const auto existing = qobject_cast<ICLEntry*> (existingObj);
		if (existing && existing->GetEntryType () != ICLEntry::EntryType::MUC)
		{
			const auto res = QMessageBox::question (nullptr,
					"LeechCraft",
					tr ("Cannot join something that's already added to the roster. "
						"Do you want to remove %1 from roster and retry?")
						.arg ("<em>" + jid + "</em>"),
					QMessageBox::Yes | QMessageBox::No);
			if (res != QMessageBox::Yes)
				return;

			RemoveEntry (existingObj);
			ExistingEntry2JoinConflict_ [existingObj] = qMakePair (jid, nick);
			return;
		}

		const auto entry = ClientConnection_->JoinRoom (jid, nick);
		if (!entry)
			return;

		if (!password.isEmpty ())
			entry->GetRoomHandler ()->GetRoom ()->setPassword (password);

		emit gotCLItems ({ entry });
	}

	void GlooxAccount::JoinRoom (const QString& server,
			const QString& room, const QString& nick, const QString& password)
	{
		const auto& jidStr = room + '@' + server;
		JoinRoom (jidStr, nick, password);
	}

	std::shared_ptr<ClientConnection> GlooxAccount::GetClientConnection () const
	{
		return ClientConnection_;
	}

	GlooxCLEntry* GlooxAccount::CreateFromODS (OfflineDataSource_ptr ods)
	{
		return ClientConnection_->AddODSCLEntry (ods);
	}

	void GlooxAccount::UpdateOurPhotoHash (const QByteArray& hash)
	{
		SettingsHolder_->SetPhotoHash (hash);
	}

	void GlooxAccount::CreateSDForResource (const QString& resource)
	{
		auto sd = new SDSession (this);
		sd->SetQuery (resource);
		emit gotSDSession (sd);
	}

	QByteArray GlooxAccount::Serialize () const
	{
		quint16 version = 9;

		QByteArray result;
		{
			QDataStream ostr (&result, QIODevice::WriteOnly);
			ostr << version
				<< Name_;
			SettingsHolder_->Serialize (ostr);
		}

		return result;
	}

	GlooxAccount* GlooxAccount::Deserialize (const QByteArray& data, GlooxProtocol *proto)
	{
		quint16 version = 0;

		QDataStream in (data);
		in >> version;

		if (version < 1 || version > 9)
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown version"
					<< version;
			return 0;
		}

		QString name;
		in >> name;
		GlooxAccount *result = new GlooxAccount (name, proto, proto);
		result->GetSettings ()->Deserialize (in, version);
		result->Init ();

		return result;
	}

	GlooxMessage* GlooxAccount::CreateMessage (IMessage::Type type,
			const QString& variant,
			const QString& body,
			const QString& jid)
	{
		return ClientConnection_->CreateMessage (type, variant, body, jid);
	}

	QString GlooxAccount::GetPassword (bool authfailure)
	{
		return ParentProtocol_->GetProxyObject ()->GetAccountPassword (this, !authfailure);
	}

	QString GlooxAccount::GetDefaultReqHost () const
	{
		const auto& second = SettingsHolder_->GetJID ().split ('@', Qt::SkipEmptyParts).value (1);
		const int slIdx = second.indexOf ('/');
		return slIdx >= 0 ? second.left (slIdx) : second;
	}

	void GlooxAccount::HandleClientConnectionAvailable (bool available)
	{
		for (auto act : GetActions ())
			act->setEnabled (available);
	}

	void GlooxAccount::handleEntryRemoved (QObject *entry)
	{
		emit removedCLItems ({ entry });

		if (ExistingEntry2JoinConflict_.contains (entry))
		{
			const auto& pair = ExistingEntry2JoinConflict_.take (entry);
			JoinRoom (pair.first, pair.second, {});
		}
	}
}
}
}
