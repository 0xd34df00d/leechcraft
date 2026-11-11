/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "misc.h"
#include <QFileInfo>
#include <QMessageBox>
#include <QWizard>
#include <interfaces/entityconstants.h>
#include <interfaces/an/constants.h>
#include <interfaces/an/entityfields.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/core/irootwindowsmanager.h>
#include <util/sll/qobjectrefcast.h>
#include <util/sll/qtutil.h>
#include <util/xpc/util.h>
#include <interfaces/azoth/iaccount.h>
#include <interfaces/azoth/iregmanagedaccount.h>
#include "components/chat/chattab.h"
#include "components/dialogs/filesenddialog.h"
#include "components/transfers/transferjobmanager.h"
#include "../../avatarsmanager.h"
#include "../../addaccountwizardfirstpage.h"
#include "../../core.h"
#include "../../chattabsmanager.h"
#include "../../hooksinstance.h"
#include "../../notificationsmanager.h"

namespace LC::Azoth
{
	bool SendMessage (ICLEntry& e, OutgoingMessage message)
	{
		bool cancel = false;
		emit HooksInstance::Instance ().messageWillBeCreated (cancel, e, message);
		if (cancel)
			return false;

		e.SendMessage (message);
		return true;
	}

	namespace
	{
		QStringList ToLocalFiles (const QList<QUrl>& urls)
		{
			QStringList result;
			result.reserve (urls.size ());
			for (const auto& url : urls)
				if (const auto& path = url.toLocalFile ();
					url.isLocalFile () && QFileInfo { path }.exists ())
					result << path;
			return result;
		}
	}

	bool OfferURLs (TransferJobManager& transfers,
			const Util::TrackingPointer<ICLEntry>& entry,
			const QList<QUrl>& urls,
			QWidget *parent)
	{
		if (!entry)
			return false;

		const auto& paths = ToLocalFiles (urls);
		if (paths.isEmpty ())
			return false;

		if (paths.size () == 1)
		{
			new FileSendDialog { &*entry, paths [0] };
			return true;
		}

		const auto& text = QObject::tr ("Are you sure you want to send %n files to %1?", nullptr, paths.size ())
				.arg (entry->GetEntryName ());
		if (QMessageBox::question (parent,
					"Azoth",
					text,
					QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
			return true;

		for (const auto& path : paths)
			transfers.SendFile ({ *entry, {}, path, {} });

		return true;
	}

	namespace
	{
		// TODO drop once the room comes from the event rather than the entry:
		// participants will be able to belong to several rooms.
		IMUCEntry* RoomOf (ICLEntry& entry)
		{
			if (entry.GetEntryType () == ICLEntry::EntryType::MUC)
				return &qobject_ref_cast<IMUCEntry> (entry.GetQObject ());

			if (const auto parent = entry.GetParentCLEntry ();
				parent && parent->GetEntryType () == ICLEntry::EntryType::MUC)
				return &qobject_ref_cast<IMUCEntry> (parent->GetQObject ());
			return nullptr;
		}
	}

	Entity BuildNotification (Entity e, NotificationInfo info)
	{
		e.Additional_ [AN::EF::SenderID] = "org.LeechCraft.Azoth";
		e.Additional_ [AN::EF::EventCategory] = AN::CatIM;
		e.Additional_ [AN::EF::EventID] = "org.LC.Plugins.Azoth." + info.EventId_ + '/' + info.Conversation_.GetEntryId ();

		e.Additional_ [AN::EF::VisualPath] = QStringList (info.Conversation_.GetEntryName ());

		int win = 0;
		const auto tab = Core::Instance ().GetChatTabsManager ()->GetChatTab (info.Conversation_.GetGlobalStrongestId ());

		const auto rootWM = GetProxyHolder ()->GetRootWindowsManager ();
		if (tab)
			win = rootWM->GetWindowForTab (tab);
		if (!tab || win == -1)
		{
			const auto& tc = info.Conversation_.GetEntryType () == ICLEntry::EntryType::MUC ?
					ChatTab::GetMUCTabClassInfo () :
					ChatTab::GetChatTabClassInfo ();
			win = rootWM->GetPreferredWindowIndex (tc.TabClass_);
		}

		e.Additional_ [AN::EF::WindowIndex] = win;

		e.Additional_.insert (NotificationsManager::SourceFields (info.Actor_));
		if (const auto room = RoomOf (info.Actor_))
			e.Additional_.insert (NotificationsManager::RoomFields (*room));

		const auto srcId = info.Actor_.GetGlobalStrongestId ();
		const auto& avatarGetter = [srcId, &mgr = info.Avatars_] { return mgr.GetAvatarImage (srcId, IAvatarsManager::NotificationSize); };
		e.Additional_ [EF::NotificationPixmap] = QVariant::fromValue<Util::LazyNotificationPixmap_t> (avatarGetter);
		return e;
	}

	void BuildAndSendNotification (Entity prefilled, NotificationInfo info)
	{
		GetProxyHolder ()->GetEntityManager ()->HandleEntity (BuildNotification (std::move (prefilled), info));
	}

	void InitiateAccountAddition (QWidget *parent)
	{
		QWizard *wizard = new QWizard (parent);
		wizard->setAttribute (Qt::WA_DeleteOnClose);
		wizard->setWindowTitle (QObject::tr ("Add account"));
		wizard->addPage (new AddAccountWizardFirstPage (wizard));

		wizard->show ();
	}

	void RemoveAccount (IAccount *acc)
	{
		if (!acc)
			return;

		if (QMessageBox::question (nullptr,
					"LeechCraft",
					QObject::tr ("Are you sure you want to remove the account %1?")
						.arg (acc->GetAccountName ()),
					QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
			return;

		if (const auto irm = qobject_cast<IRegManagedAccount*> (acc->GetQObject ()))
			if (irm->SupportsFeature (IRegManagedAccount::Feature::DeregisterAcc) &&
					QMessageBox::question (nullptr,
							"LeechCraft",
							QObject::tr ("Do you also want to remove %1 from the server?")
								.arg (acc->GetAccountName ()),
							QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
			{
				irm->DeregisterAccount ();
				return;
			}

		auto& proto = qobject_ref_cast<IProtocol> (acc->GetParentProtocol ());
		proto.RemoveAccount (acc->GetQObject ());
	}
}
