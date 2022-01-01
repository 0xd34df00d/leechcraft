/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "vkaccount.h"
#include <stdexcept>
#include <QUuid>
#include <QIcon>
#include <QtDebug>
#include <util/svcauth/vkcaptchadialog.h>
#include <util/sll/prelude.h>
#include <util/sll/either.h>
#include <util/sll/unreachable.h>
#include "vkprotocol.h"
#include "vkconnection.h"
#include "vkentry.h"
#include "vkmessage.h"
#include "photofetcher.h"
#include "georesolver.h"
#include "groupsmanager.h"
#include "xmlsettingsmanager.h"
#include "vkchatentry.h"
#include "logger.h"
#include "accountconfigdialog.h"
#include "serverhistorymanager.h"
#include "vkconnectiontunesetter.h"
#include "transfermanager.h"
#include "appinfomanager.h"

namespace LC
{
namespace Azoth
{
namespace Murm
{
	VkAccount::VkAccount (const QString& name, VkProtocol *proto,
			ICoreProxy_ptr proxy, const QByteArray& id, const QByteArray& cookies)
	: QObject (proto)
	, CoreProxy_ (proxy)
	, Proto_ (proto)
	, ID_ (id.isEmpty () ? QUuid::createUuid ().toByteArray () : id)
	, PhotoStorage_ (new PhotoFetcher (proxy->GetNetworkAccessManager (), this))
	, Name_ (name)
	, Logger_ (new Logger (ID_, this))
	, Conn_ (new VkConnection (name, cookies, proxy, *Logger_))
	, ConnTuneSetter_ (new VkConnectionTuneSetter (Conn_, proxy))
	, GroupsMgr_ (new GroupsManager (Conn_))
	, GeoResolver_ (new GeoResolver (Conn_, this))
	, ServHistMgr_ (new ServerHistoryManager (this))
	, XFerMgr_ (new TransferManager (this))
	, AppInfoMgr_ (new AppInfoManager (proxy->GetNetworkAccessManager (), Conn_, this))
	{
		connect (Conn_,
				SIGNAL (cookiesChanged ()),
				this,
				SLOT (emitUpdateAcc ()));
		connect (Conn_,
				SIGNAL (stoppedPolling ()),
				this,
				SLOT (finishOffline ()));

		connect (Conn_,
				SIGNAL (gotSelfInfo (UserInfo)),
				this,
				SLOT (handleSelfInfo (UserInfo)));
		connect (Conn_,
				SIGNAL (gotUsers (QList<UserInfo>)),
				this,
				SLOT (handleUsers (QList<UserInfo>)));
		connect (Conn_,
				SIGNAL (gotNRIList (QList<qulonglong>)),
				this,
				SLOT (handleNRIList (QList<qulonglong>)));
		connect (Conn_,
				SIGNAL (userStateChanged (qulonglong, bool)),
				this,
				SLOT (handleUserState (qulonglong, bool)));
		connect (Conn_,
				SIGNAL (gotUserAppInfoStub (qulonglong, AppInfo)),
				this,
				SLOT (handleUserAppInfoStub (qulonglong, AppInfo)));

		connect (Conn_,
				SIGNAL (gotMessage (MessageInfo)),
				this,
				SLOT (handleMessage (MessageInfo)));
		connect (Conn_,
				SIGNAL (gotMessage (FullMessageInfo, MessageInfo)),
				this,
				SLOT (handleMessage (FullMessageInfo, MessageInfo)));
		connect (Conn_,
				SIGNAL (gotTypingNotification (qulonglong)),
				this,
				SLOT (handleTypingNotification (qulonglong)));
		connect (Conn_,
				SIGNAL (statusChanged (EntryStatus)),
				this,
				SIGNAL (statusChanged (EntryStatus)));

		connect (Conn_,
				SIGNAL (mucChanged (qulonglong)),
				this,
				SLOT (handleMucChanged (qulonglong)));
		connect (Conn_,
				SIGNAL (gotChatInfo (ChatInfo)),
				this,
				SLOT (handleGotChatInfo (ChatInfo)));
		connect (Conn_,
				SIGNAL (chatUserRemoved (qulonglong, qulonglong)),
				this,
				SLOT (handleChatUserRemoved (qulonglong, qulonglong)));

		connect (Conn_,
				SIGNAL (captchaNeeded (QString, QUrl)),
				this,
				SLOT (handleCaptcha (QString, QUrl)));

		connect (Logger_,
				SIGNAL (gotConsolePacket (QByteArray, IHaveConsole::PacketDirection, QString)),
				this,
				SIGNAL (gotConsolePacket (QByteArray, IHaveConsole::PacketDirection, QString)));

		connect (ServHistMgr_,
				SIGNAL (serverHistoryFetched (QModelIndex, QByteArray, SrvHistMessages_t)),
				this,
				SIGNAL (serverHistoryFetched (QModelIndex, QByteArray, SrvHistMessages_t)));

		connect (AppInfoMgr_,
				SIGNAL (gotAppInfo (AppInfo)),
				this,
				SLOT (handleAppInfo (AppInfo)));
	}

	QByteArray VkAccount::Serialize () const
	{
		QByteArray result;
		QDataStream out (&result, QIODevice::WriteOnly);

		out << static_cast<quint8> (4)
				<< ID_
				<< Name_
				<< Conn_->GetCookies ()
				<< EnableFileLog_
				<< PublishTune_
				<< MarkAsOnline_
				<< UpdateStatus_;

		return result;
	}

	VkAccount* VkAccount::Deserialize (const QByteArray& data, VkProtocol *proto, ICoreProxy_ptr proxy)
	{
		QDataStream in (data);

		quint8 version = 0;
		in >> version;
		if (version < 1 || version > 4)
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown version"
					<< version;
			return nullptr;
		}

		QByteArray id;
		QString name;
		QByteArray cookies;

		in >> id
				>> name
				>> cookies;

		auto acc = new VkAccount (name, proto, proxy, id, cookies);

		if (version >= 2)
			in >> acc->EnableFileLog_
					>> acc->PublishTune_;
		if (version >= 3)
			in >> acc->MarkAsOnline_;
		if (version >= 4)
			in >> acc->UpdateStatus_;

		acc->Init ();

		return acc;
	}

	void VkAccount::Init ()
	{
		Logger_->SetFileEnabled (EnableFileLog_);
		handleMarkOnline ();
	}

	void VkAccount::Send (qulonglong to, VkConnection::Type type,
			VkMessage *msg, const QByteArrayList& attachments)
	{
		QPointer<VkMessage> safeMsg { msg };
		Conn_->SendMessage (to,
				msg->GetRawBody (),
				[safeMsg] (qulonglong id)
				{
					if (safeMsg)
						safeMsg->SetID (id);
				},
				type,
				attachments);
	}

	void VkAccount::CreateChat (const QString& name, const QList<VkEntry*>& entries)
	{
		QList<qulonglong> ids;
		for (auto entry : entries)
			ids << entry->GetInfo ().ID_;
		Conn_->CreateChat (name, ids);
	}

	VkEntry* VkAccount::GetEntry (qulonglong id) const
	{
		return Entries_.value (id);
	}

	VkEntry* VkAccount::FindEntryById (const QString& entryId) const
	{
		const auto pos = std::find_if (Entries_.begin (), Entries_.end (),
				[&entryId] (VkEntry *entry) { return entry->GetEntryID () == entryId; });
		return pos != Entries_.end () ? *pos : nullptr;
	}

	VkEntry* VkAccount::GetSelf () const
	{
		return SelfEntry_;
	}

	VkEntry* VkAccount::GetEntryOrCreate (const UserInfo& info)
	{
		if (!Entries_.contains (info.ID_))
			CreateUsers ({ info });

		return Entries_.value (info.ID_);
	}

	ICoreProxy_ptr VkAccount::GetCoreProxy () const
	{
		return CoreProxy_;
	}

	VkConnection* VkAccount::GetConnection () const
	{
		return Conn_;
	}

	PhotoFetcher* VkAccount::GetPhotoStorage () const
	{
		return PhotoStorage_;
	}

	GeoResolver* VkAccount::GetGeoResolver () const
	{
		return GeoResolver_;
	}

	GroupsManager* VkAccount::GetGroupsManager () const
	{
		return GroupsMgr_;
	}

	Logger& VkAccount::GetLogger () const
	{
		return *Logger_;
	}

	QObject* VkAccount::GetQObject ()
	{
		return this;
	}

	VkProtocol* VkAccount::GetParentProtocol () const
	{
		return Proto_;
	}

	IAccount::AccountFeatures VkAccount::GetAccountFeatures () const
	{
		return AccountFeature::FRenamable;
	}

	QList<QObject*> VkAccount::GetCLEntries ()
	{
		QList<QObject*> result;
		result.reserve (Entries_.size () + ChatEntries_.size ());
		std::copy (Entries_.begin (), Entries_.end (), std::back_inserter (result));
		std::copy (ChatEntries_.begin (), ChatEntries_.end (), std::back_inserter (result));
		return result;
	}

	QString VkAccount::GetAccountName () const
	{
		return Name_;
	}

	QString VkAccount::GetOurNick () const
	{
		return tr ("me");
	}

	void VkAccount::RenameAccount (const QString& name)
	{
		Name_ = name;
		emit accountRenamed (name);
		emit accountChanged (this);
	}

	QByteArray VkAccount::GetAccountID () const
	{
		return ID_;
	}

	QList<QAction*> VkAccount::GetActions () const
	{
		return {};
	}

	void VkAccount::OpenConfigurationDialog ()
	{
		auto dia = new AccountConfigDialog;

		AccConfigDia_ = dia;

		dia->SetFileLogEnabled (EnableFileLog_);
		dia->SetPublishTuneEnabled (PublishTune_);
		dia->SetMarkAsOnline (MarkAsOnline_);
		dia->SetUpdateStatusEnabled (UpdateStatus_);

		connect (dia,
				SIGNAL (reauthRequested ()),
				Conn_,
				SLOT (reauth ()));

		connect (dia,
				SIGNAL (rejected ()),
				dia,
				SLOT (deleteLater ()));
		connect (dia,
				SIGNAL (accepted ()),
				this,
				SLOT (handleConfigDialogAccepted ()));

		dia->show ();
	}

	EntryStatus VkAccount::GetState () const
	{
		return Conn_->GetStatus ();
	}

	void VkAccount::ChangeState (const EntryStatus& status)
	{
		Conn_->SetStatus (status, UpdateStatus_);
	}

	void VkAccount::Authorize (QObject*)
	{
	}

	void VkAccount::DenyAuth (QObject*)
	{
	}

	void VkAccount::RequestAuth (const QString&, const QString&, const QString&, const QStringList&)
	{
	}

	void VkAccount::RemoveEntry (QObject *entryObj)
	{
		auto entry = qobject_cast<VkEntry*> (entryObj);
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< entry
					<< "is not a VkEntry";
			return;
		}

		if (entry->IsNonRoster ())
		{
			emit removedCLItems ({ entry });

			const auto id = entry->GetInfo ().ID_;
			Entries_.remove (id);
			entry->deleteLater ();

			NonRosterItems_.removeOne (id);
			Conn_->SetNRIList (NonRosterItems_);

			return;
		}
	}

	QObject* VkAccount::GetTransferManager () const
	{
		return XFerMgr_;
	}

	void VkAccount::PublishTune (const QMap<QString, QVariant>& tuneData)
	{
		if (!PublishTune_)
			return;

		ConnTuneSetter_->SetTune (tuneData);
	}

	QObject* VkAccount::GetSelfContact () const
	{
		return SelfEntry_;
	}

	QIcon VkAccount::GetAccountIcon () const
	{
		return {};
	}

	IHaveConsole::PacketFormat VkAccount::GetPacketFormat () const
	{
		return PacketFormat::PlainText;
	}

	void VkAccount::SetConsoleEnabled (bool)
	{
	}

	QObject* VkAccount::CreateNonRosterItem (const QString& idStr)
	{
		auto realId = idStr;
		if (realId.startsWith ("id"))
			realId = realId.remove (0, 2);

		bool ok = false;
		const auto id = realId.toULongLong (&ok);
		if (!ok)
			throw std::runtime_error (tr ("%1 is invalid VKontake ID")
					.arg (idStr)
					.toUtf8 ().constData ());

		if (Entries_.contains (id))
			return Entries_ [id];

		const auto entry = CreateNonRosterItem (id);
		emit gotCLItems ({ entry });

		NonRosterItems_ << id;
		Conn_->SetNRIList (NonRosterItems_);
		Conn_->GetUserInfo ({ id });

		return entry;
	}

	bool VkAccount::HasFeature (ServerHistoryFeature feature) const
	{
		switch (feature)
		{
		case ServerHistoryFeature::AccountSupportsHistory:
		case ServerHistoryFeature::DatedFetching:
			return true;
		case ServerHistoryFeature::Configurable:
			return false;
		}

		Util::Unreachable ();
	}

	void VkAccount::OpenServerHistoryConfiguration ()
	{
	}

	QAbstractItemModel* VkAccount::GetServerContactsModel () const
	{
		return ServHistMgr_->GetModel ();
	}

	void VkAccount::FetchServerHistory (const QModelIndex& contact, const QByteArray& startId, int count)
	{
		ServHistMgr_->RequestHistory (contact, startId.toInt (), count);
	}

	DefaultSortParams VkAccount::GetSortParams () const
	{
		return { 0, ServerHistoryRole::LastMessageDate, Qt::DescendingOrder };
	}

	QFuture<IHaveServerHistory::DatedFetchResult_t> VkAccount::FetchServerHistory (const QDateTime& since)
	{
		return ServHistMgr_->FetchServerHistory (since);
	}

	void VkAccount::TryPendingMessages ()
	{
		decltype (PendingMessages_) pending;

		using std::swap;
		swap (pending, PendingMessages_);

		for (const auto& pair : pending)
			handleMessage (pair.second, pair.first);
	}

	VkEntry* VkAccount::CreateNonRosterItem (qulonglong id)
	{
		UserInfo info;
		info.ID_ = id;

		auto entry = new VkEntry (info, this);
		entry->SetNonRoster ();
		Entries_ [id] = entry;

		return entry;
	}

	bool VkAccount::CreateUsers (const QList<UserInfo>& infos)
	{
		AppInfoMgr_->CacheAppInfo (Util::Map (infos, &UserInfo::AppInfo_));

		QList<QObject*> newEntries;
		QHash<int, QString> newCountries;
		QHash<int, QString> newCities;
		bool hadNew = false;
		for (const auto& info : infos)
		{
			if (Entries_.contains (info.ID_))
			{
				const auto entry = Entries_ [info.ID_];
				entry->UpdateInfo (info);

				continue;
			}

			auto entry = new VkEntry (info, this);
			Entries_ [info.ID_] = entry;
			newEntries << entry;

			newCountries [info.Country_] = info.CountryName_;
			newCities [info.City_] = info.CityName_;

			hadNew = true;
		}

		GeoResolver_->AddCountriesToCache (newCountries);
		GeoResolver_->AddCitiesToCache (newCities);

		if (!newEntries.isEmpty ())
			emit gotCLItems (newEntries);

		return hadNew;
	}

	void VkAccount::handleSelfInfo (const UserInfo& info)
	{
		CreateUsers ({ info });

		SelfEntry_ = Entries_ [info.ID_];
		SelfEntry_->SetSelf ();
	}

	void VkAccount::handleUsers (const QList<UserInfo>& infos)
	{
		if (CreateUsers (infos))
		{
			TryPendingMessages ();
			ServHistMgr_->refresh ();
		}
	}

	void VkAccount::handleNRIList (const QList<qulonglong>& ids)
	{
		QList<qulonglong> toRequest;
		QList<QObject*> objs;
		for (auto id : ids)
		{
			if (Entries_.contains (id))
				continue;

			toRequest << id;
			objs << CreateNonRosterItem (id);
		}

		emit gotCLItems (objs);
		Conn_->GetUserInfo (toRequest);

		NonRosterItems_ = toRequest;
	}

	void VkAccount::handleUserState (qulonglong id, bool isOnline)
	{
		if (!Entries_.contains (id))
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown user"
					<< id;
			Conn_->RerequestFriends ();
			return;
		}

		auto entry = Entries_.value (id);
		auto info = entry->GetInfo ();
		info.IsOnline_ = isOnline;
		entry->UpdateInfo (info);

		entry->UpdateAppInfo ({}, {});

		if (isOnline)
			Conn_->RequestUserAppId (id);
	}

	void VkAccount::handleUserAppInfoStub (qulonglong id, const AppInfo& appInfo)
	{
		if (!Entries_.contains (id))
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown user"
					<< id;
			return;
		}

		const auto entry = Entries_.value (id);

		const auto appId = appInfo.AppId_;
		AppInfoMgr_->PerformWithAppInfo (appId,
				[this, entry] (const AppInfo& info)
					{ entry->UpdateAppInfo (info, AppInfoMgr_->GetAppImage (info)); },
				[entry, appInfo] { entry->UpdateAppInfo (appInfo, {}); });
	}

	void VkAccount::handleMessage (const MessageInfo& info)
	{
		handleMessage ({}, info);
	}

	void VkAccount::handleMessage (const FullMessageInfo& fullInfo, const MessageInfo& info)
	{
		if (info.Params_.value ("source_act") == "chat_kick_user" &&
				info.Params_.value ("source_mid").toULongLong () == SelfEntry_->GetInfo ().ID_)
			return;

		const auto from = info.From_;
		if (info.Flags_ & MessageFlag::Chat)
		{
			if (!ChatEntries_.contains (from))
			{
				qDebug () << Q_FUNC_INFO
						<< "unknown chat entry"
						<< from;

				if (!std::any_of (PendingMessages_.begin (), PendingMessages_.end (),
						[from] (const QPair<MessageInfo, FullMessageInfo>& info)
						{
							return from == info.first.From_;
						}))
				{
					qDebug () << Q_FUNC_INFO
							<< "requesting info for"
							<< from;
					Conn_->RequestChatInfo (from);
				}

				PendingMessages_.append ({ info, fullInfo });
				return;
			}

			switch (ChatEntries_.value (from)->HandleMessage (info, fullInfo))
			{
			case VkChatEntry::HandleMessageResult::Accepted:
			case VkChatEntry::HandleMessageResult::Rejected:
				return;
			case VkChatEntry::HandleMessageResult::UserInfoRequested:
				PendingMessages_.append ({ info, fullInfo });
				return;
			}
		}
		else
		{
			if (!Entries_.contains (from))
			{
				qWarning () << Q_FUNC_INFO
						<< "message from unknown user"
						<< from;

				PendingMessages_.append ({ info, fullInfo });

				Conn_->GetUserInfo ({ from });
				return;
			}

			Entries_.value (from)->HandleMessage (info, fullInfo);
		}
	}

	void VkAccount::handleTypingNotification (qulonglong uid)
	{
		if (!Entries_.contains (uid))
		{
			qWarning () << Q_FUNC_INFO
					<< "message from unknown user"
					<< uid;
			Conn_->RerequestFriends ();
			return;
		}

		const auto entry = Entries_.value (uid);
		entry->HandleTypingNotification ();
	}

	void VkAccount::handleMarkOnline ()
	{
		Conn_->SetMarkingOnlineEnabled (MarkAsOnline_);
	}

	void VkAccount::finishOffline ()
	{
		if (!ChatEntries_.isEmpty ())
		{
			QList<QObject*> toRemove;
			for (auto item : ChatEntries_)
				toRemove << item;
			emit removedCLItems (toRemove);

			qDeleteAll (ChatEntries_);
			ChatEntries_.clear ();
		}

		for (auto entry : Entries_)
		{
			auto info = entry->GetInfo ();
			info.IsOnline_ = false;
			entry->UpdateInfo (info, false);
		}
	}

	void VkAccount::handleCaptcha (const QString& cid, const QUrl& url)
	{
		if (IsRequestingCaptcha_)
		{
			Conn_->HandleCaptcha (cid, {});
			return;
		}

		auto dia = new Util::SvcAuth::VkCaptchaDialog (url, cid, CoreProxy_->GetNetworkAccessManager ());
		dia->SetContextName ("Azoth Murm");
		connect (dia,
				SIGNAL (gotCaptcha (QString, QString)),
				this,
				SLOT (handleCaptchaEntered (QString, QString)));
		dia->show ();

		IsRequestingCaptcha_ = true;
	}

	void VkAccount::handleCaptchaEntered (const QString& cid, const QString& value)
	{
		Conn_->HandleCaptcha (cid, value);
		IsRequestingCaptcha_ = false;
	}

	void VkAccount::handleConfigDialogAccepted()
	{
		if (!AccConfigDia_)
			return;

		EnableFileLog_ = AccConfigDia_->GetFileLogEnabled ();
		Logger_->SetFileEnabled (EnableFileLog_);

		MarkAsOnline_ = AccConfigDia_->GetMarkAsOnline ();
		handleMarkOnline ();

		PublishTune_ = AccConfigDia_->GetPublishTuneEnabled ();

		UpdateStatus_ = AccConfigDia_->GetUpdateStatusEnabled ();

		emit accountChanged (this);

		AccConfigDia_->deleteLater ();
	}

	void VkAccount::handleMucChanged (qulonglong chat)
	{
		if (!ChatEntries_.contains (chat))
		{
			qDebug () << Q_FUNC_INFO
					<< "ignoring chat change for non-present chat"
					<< chat;
			return;
		}

		Conn_->RequestChatInfo (chat);
	}

	void VkAccount::handleGotChatInfo (const ChatInfo& info)
	{
		if (!ChatEntries_.contains (info.ChatID_))
		{
			auto entry = new VkChatEntry (info, this);
			connect (entry,
					&VkChatEntry::removeEntry,
					this,
					[entry, this]
					{
						ChatEntries_.remove (ChatEntries_.key (entry));
						emit removedCLItems ({ entry });
						entry->deleteLater ();
					});
			ChatEntries_ [info.ChatID_] = entry;
			emit gotCLItems ({ entry });

			TryPendingMessages ();
		}
		else
			ChatEntries_ [info.ChatID_]->UpdateInfo (info);
	}

	void VkAccount::handleChatUserRemoved (qulonglong chat, qulonglong id)
	{
		if (!ChatEntries_.contains (chat))
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown chat"
					<< chat;
			return;
		}

		ChatEntries_ [chat]->HandleRemoved (id);
	}

	void VkAccount::handleAppInfo (const AppInfo& info)
	{
		for (const auto entry : Entries_)
			if (entry->GetInfo ().AppInfo_.AppId_ == info.AppId_)
				entry->UpdateAppInfo (info, AppInfoMgr_->GetAppImage (info));
	}

	void VkAccount::emitUpdateAcc ()
	{
		emit accountChanged (this);
	}
}
}
}
