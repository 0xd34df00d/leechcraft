/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "misc.h"
#include <QFileInfo>
#include <QFuture>
#include <QMessageBox>
#include <QWizard>
#include <interfaces/an/constants.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/irootwindowsmanager.h>
#include <util/xpc/util.h>
#include <interfaces/azoth/iaccount.h>
#include <interfaces/azoth/iregmanagedaccount.h>
#include "components/chat/chattab.h"
#include "components/dialogs/filesenddialog.h"
#include "../../avatarsmanager.h"
#include "../../addaccountwizardfirstpage.h"
#include "../../core.h"
#include "../../chattabsmanager.h"
#include "../../hooksinstance.h"
#include "../../transferjobmanager.h"

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
		void CleanupUrls (QList<QUrl>& urls)
		{
			for (auto i = urls.begin (); i != urls.end (); )
				if (!i->isLocalFile ())
					i = urls.erase (i);
				else
					++i;
		}
	}

	bool OfferURLs (TransferJobManager& transfers, ICLEntry *entry, QList<QUrl> urls, QWidget *parent)
	{
		if (!entry)
			return false;

		CleanupUrls (urls);
		if (urls.isEmpty ())
			return false;

		if (urls.size () == 1)
		{
			new FileSendDialog { entry, urls.value (0).toLocalFile () };
			return true;
		}

		const auto& text = QObject::tr ("Are you sure you want to send %n files to %1?", 0, urls.size ())
				.arg (entry->GetEntryName ());
		if (QMessageBox::question (parent,
					"Azoth",
					text,
					QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
			return false;

		bool allSuccess = true;
		for (const auto& url : urls)
			if (const auto& path = url.toLocalFile ();
				QFileInfo { path }.exists ())
				allSuccess = transfers.SendFile ({ *entry, {}, path, {} }) && allSuccess;

		return allSuccess;
	}

	QFuture<Entity> BuildNotification (AvatarsManager *avatarsMgr,
			Entity e, ICLEntry *other, const QString& id, ICLEntry *avatarSource)
	{
		e.Additional_ ["org.LC.AdvNotifications.SenderID"] = "org.LeechCraft.Azoth";
		e.Additional_ ["org.LC.AdvNotifications.EventCategory"] = AN::CatIM;
		e.Additional_ ["org.LC.AdvNotifications.EventID"] =
				"org.LC.Plugins.Azoth." + (id.isEmpty () ? "IncomingMessageFrom/" : id) + other->GetEntryID ();

		e.Additional_ ["org.LC.AdvNotifications.VisualPath"] = QStringList (other->GetEntryName ());

		int win = 0;
		const auto tab = Core::Instance ().GetChatTabsManager ()->GetChatTab (other->GetEntryID ());

		const auto rootWM = GetProxyHolder ()->GetRootWindowsManager ();
		if (tab)
			win = rootWM->GetWindowForTab (tab);
		if (!tab || win == -1)
		{
			const auto& tc = other->GetEntryType () == ICLEntry::EntryType::MUC ?
					ChatTab::GetMUCTabClassInfo () :
					ChatTab::GetChatTabClassInfo ();
			win = rootWM->GetPreferredWindowIndex (tc.TabClass_);
		}

		e.Additional_ ["org.LC.AdvNotifications.WindowIndex"] = win;

		e.Additional_ ["org.LC.Plugins.Azoth.SourceName"] = other->GetEntryName ();
		e.Additional_ ["org.LC.Plugins.Azoth.SourceID"] = other->GetEntryID ();
		e.Additional_ ["org.LC.Plugins.Azoth.SourceGroups"] = other->Groups ();

		if (const auto parent = other->GetParentCLEntry ())
		{
			e.Additional_ ["org.LC.Plugins.Azoth.ParentSourceID"] = parent->GetEntryID ();
			e.Additional_ ["org.LC.Plugins.Azoth.ParentSourceName"] = parent->GetEntryName ();
		}

		if (!avatarSource)
			avatarSource = other;

		QPointer<AvatarsManager> mgr { avatarsMgr };
		QPointer<QObject> src { avatarSource->GetQObject () };
		const auto& avatarFutureGetter = [mgr, src] () -> Util::LazyNotificationPixmap_t::result_type
		{
			if (mgr && src)
				return mgr->GetAvatar (src, IHaveAvatars::Size::Thumbnail);
			else
				return {};
		};
		e.Additional_ ["NotificationPixmap"] = QVariant::fromValue<Util::LazyNotificationPixmap_t> (avatarFutureGetter);
		return QtFuture::makeReadyValueFuture (e);
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

		auto protoObj = acc->GetParentProtocol ();
		auto proto = qobject_cast<IProtocol*> (protoObj);
		if (!proto)
		{
			qWarning () << Q_FUNC_INFO
					<< "parent protocol for"
					<< acc->GetAccountID ()
					<< "doesn't implement IProtocol";
			return;
		}
		proto->RemoveAccount (acc->GetQObject ());
	}
}
