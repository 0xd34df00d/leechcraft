/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "accountactions.h"
#include <QAction>
#include <QMenu>
#include <QHash>
#include <QInputDialog>
#include <QMainWindow>
#include <QMessageBox>
#include <QtDebug>
#include <interfaces/core/irootwindowsmanager.h>
#include <util/util.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/azoth/iextselfinfoaccount.h>
#include "interfaces/azoth/iaccount.h"
#include "interfaces/azoth/iaccountactionsprovider.h"
#include "interfaces/azoth/imucjoinwidget.h"
#include "interfaces/azoth/iprotocol.h"
#include "interfaces/azoth/isupportbookmarks.h"
#include "interfaces/azoth/isupportactivity.h"
#include "interfaces/azoth/isupportmood.h"
#include "interfaces/azoth/isupportgeolocation.h"
#include "interfaces/azoth/ihaveservicediscovery.h"
#include "interfaces/azoth/ihaveconsole.h"
#include "interfaces/azoth/ihavemicroblogs.h"
#include "interfaces/azoth/iregmanagedaccount.h"
#include "interfaces/azoth/isupportnonroster.h"
#include "interfaces/azoth/ihaveserverhistory.h"
#include "interfaces/azoth/imucprotocol.h"
#include "interfaces/azoth/ihavecontactmood.h"
#include "interfaces/azoth/ihavecontactactivity.h"
#include "interfaces/azoth/moodinfo.h"
#include "interfaces/azoth/activityinfo.h"
#include "core.h"
#include "joinconferencedialog.h"
#include "bookmarksmanagerdialog.h"
#include "addcontactdialog.h"
#include "activitydialog.h"
#include "mooddialog.h"
#include "locationdialog.h"
#include "consolewidget.h"
#include "servicediscoverywidget.h"
#include "microblogstab.h"
#include "chattabsmanager.h"
#include "statuschange.h"
#include "setstatusdialog.h"
#include "xmlsettingsmanager.h"
#include "serverhistorywidget.h"
#include "resourcesmanager.h"
#include "util.h"

namespace LC::Azoth::Actions
{
	namespace
	{
		QWidget* GetDialogParentWidget ()
		{
			return GetProxyHolder ()->GetRootWindowsManager ()->GetPreferredWindow ();
		}

		void ChangeStatus (IAccount *acc, State state, const QString& text)
		{
			EntryStatus status;
			if (state != SInvalid)
				status = EntryStatus { state, StatusChange::GetStatusText (state, text) };
			else
			{
				SetStatusDialog ssd { acc->GetAccountID (), GetDialogParentWidget () };
				if (ssd.exec () != QDialog::Accepted)
					return;

				status = EntryStatus { ssd.GetState (), ssd.GetStatusText () };
			}

			acc->ChangeState (status);
		}

		void AddBookmarkedMucs (QMenu *menu, IAccount *account)
		{
			const auto isb = qobject_cast<ISupportBookmarks*> (account->GetQObject ());
			const auto mucProto = qobject_cast<IMUCProtocol*> (account->GetParentProtocol ());
			if (!isb || !mucProto)
				return;

			auto bms = isb->GetBookmarkedMUCs ();
			for (auto mucObj : account->GetCLEntries ())
				if (const auto muc = qobject_cast<IMUCEntry*> (mucObj))
					bms.removeOne (muc->GetIdentifyingData ());
			if (bms.isEmpty ())
				return;

			const auto bmsMenu = menu->addMenu (AccountActions::tr ("Join bookmarked conference"));
			for (const auto& bm : bms)
			{
				const QVariantMap& bmData = bm.toMap ();
				if (bmData.isEmpty ())
					continue;

				auto name = bmData ["StoredName"].toString ();
				const auto& hrName = bmData ["HumanReadableName"].toString ();
				if (name.isEmpty ())
					name = hrName;

				const auto act = bmsMenu->addAction (name,
						[account, mucProto, bmData]
						{
							const auto jWidget = mucProto->GetMUCJoinWidget ();
							const auto imjw = qobject_cast<IMUCJoinWidget*> (jWidget);
							imjw->SetIdentifyingData (bmData);
							imjw->Join (account->GetQObject ());

							jWidget->deleteLater ();
						});
				act->setToolTip (hrName);
			}
		}

		void AddConferenceActions (QMenu *menu, IAccount *account)
		{
			const auto itm = GetProxyHolder ()->GetIconThemeManager ();

			menu->addAction (itm->GetIcon ("irc-join-channel"),
					AccountActions::tr ("Join conference..."),
					[account]
					{
						auto dia = new JoinConferenceDialog ({ account }, GetDialogParentWidget ());
						dia->show ();
						dia->setAttribute (Qt::WA_DeleteOnClose, true);
					});

			AddBookmarkedMucs (menu, account);

			menu->addAction (itm->GetIcon ("bookmarks-organize"),
					AccountActions::tr ("Manage bookmarks..."),
					[account]
					{
						auto dia = new BookmarksManagerDialog (GetDialogParentWidget ());
						dia->FocusOn (account);
						dia->show ();
					});

			menu->addSeparator ();
		}

		void AddAccountContact (IAccount *account)
		{
			AddContactDialog dia { account, GetDialogParentWidget () };
			if (dia.exec () == QDialog::Accepted)
				dia.GetSelectedAccount ()->RequestAuth (dia.GetContactID (),
						dia.GetReason (), dia.GetNick (), dia.GetGroups ());
		}

		void OpenNonRosterChat (ISupportNonRoster *isnr)
		{
			QObject *entryObj = nullptr;
			QString contactId;
			while (!entryObj)
			{
				contactId = QInputDialog::getText (nullptr,
						AccountActions::tr ("Open chat with non-roster contact"),
						AccountActions::tr ("Enter ID of the contact you wish to open chat with:"),
						QLineEdit::Normal,
						contactId);
				if (contactId.isEmpty ())
					return;

				try
				{
					entryObj = isnr->CreateNonRosterItem (contactId);
				}
				catch (const std::exception& e)
				{
					QMessageBox::critical (nullptr,
							"LeechCraft Azoth",
							AccountActions::tr ("Error opening chat: %1")
								.arg (QString::fromUtf8 (e.what ())));
				}
			}

			const auto entry = qobject_cast<ICLEntry*> (entryObj);
			Core::Instance ().GetChatTabsManager ()->OpenChat (entry, true);
		}

		void AddServerHistoryActions (QMenu *menu, IHaveServerHistory *ihsh)
		{
			if (ihsh->HasFeature (ServerHistoryFeature::AccountSupportsHistory))
				menu->addAction (GetProxyHolder ()->GetIconThemeManager ()->GetIcon ("network-server-database"),
						AccountActions::tr ("Open server history..."),
						[ihsh]
						{
							GetProxyHolder ()->GetRootWindowsManager ()->AddTab (new ServerHistoryWidget { ihsh });
						});

			if (ihsh->HasFeature (ServerHistoryFeature::Configurable))
				menu->addAction (AccountActions::tr ("Configure server history..."),
						[ihsh] { ihsh->OpenServerHistoryConfiguration (); });

			menu->addSeparator ();
		}

		template<typename R, typename C, typename F>
		void InitSelfDialog (IExtSelfInfoAccount *iesi, R (C::*getter) (const QString&) const, F act)
		{
			const auto selfContact = iesi ? iesi->GetSelfContact () : nullptr;
			if (const auto iface = qobject_cast<C*> (selfContact))
				act ((iface->*getter) ({}));
		}

		void AddExtendedStatusIcons (QMenu *menu, QObject *accObj)
		{
			const auto iesi = qobject_cast<IExtSelfInfoAccount*> (accObj);

			if (const auto isa = qobject_cast<ISupportActivity*> (accObj))
				menu->addAction (GetProxyHolder ()->GetIconThemeManager ()->GetIcon ("face-smile"),
						AccountActions::tr("Set activity..."),
						[=]
						{
							ActivityDialog dia { GetDialogParentWidget () };
							InitSelfDialog (iesi, &IHaveContactActivity::GetUserActivity,
									[&dia] (const ActivityInfo& info) { dia.SetActivityInfo (info); });
							if (dia.exec () == QDialog::Accepted)
								isa->SetActivity (dia.GetActivityInfo ());
						});

			if (const auto ism = qobject_cast<ISupportMood*> (accObj))
				menu->addAction (AccountActions::tr ("Set mood..."),
						[=]
						{
							MoodDialog dia { GetDialogParentWidget () };
							InitSelfDialog (iesi, &IHaveContactMood::GetUserMood,
									[&dia] (const MoodInfo& info) { dia.SetMood (info); });
							if (dia.exec () == QDialog::Accepted)
								ism->SetMood (dia.GetMood ());
						});

			if (const auto isg = qobject_cast<ISupportGeolocation*> (accObj))
				menu->addAction (AccountActions::tr ("Set location..."),
						[isg]
						{
							LocationDialog dia { GetDialogParentWidget () };
							if (dia.exec () == QDialog::Accepted)
								isg->SetGeolocationInfo (dia.GetInfo ());
						});

			menu->addSeparator ();
		}

		void OpenConsole (IAccount *account)
		{
			static QHash<IAccount*, ConsoleWidget*> account2cw;

			auto& cw = account2cw [account];
			if (!cw)
			{
				cw = new ConsoleWidget (account->GetQObject ());
				QObject::connect (cw,
						&ConsoleWidget::removeTab,
						[account] { account2cw.remove (account); });
			}

			GetProxyHolder ()->GetRootWindowsManager ()->AddTab (cw->GetTitle (), cw);
		}

		void UpdatePassword (IAccount *account, IRegManagedAccount *irma)
		{
			const auto& pass = QInputDialog::getText (nullptr,
					AccountActions::tr ("Change password"),
					AccountActions::tr ("Enter new password for account %1 (the password will be updated on server):")
						.arg (account->GetAccountName ()),
					QLineEdit::Password);
			if (!pass.isEmpty ())
				irma->UpdateServerPassword (pass);
		}

		void RenameAccount (IAccount *account)
		{
			const auto& name = account->GetAccountName ();
			const auto& newName = QInputDialog::getText (nullptr,
					AccountActions::tr ("Rename account"),
					AccountActions::tr ("Enter new name for account %1:")
						.arg (name),
					QLineEdit::Normal,
					name);
			if (!newName.isEmpty ())
				account->RenameAccount (newName);
		}
	}

	void PopulateMenu (QMenu *menu, IAccount *acc)
	{
		const auto accObj = acc->GetQObject ();
		const auto proto = qobject_cast<IProtocol*> (acc->GetParentProtocol ());

		const auto itm = GetProxyHolder ()->GetIconThemeManager ();

		const auto statusMenu = StatusChange::CreateMenu (menu, std::bind_front (ChangeStatus, acc));
		statusMenu->setIcon (itm->GetIcon ("im-status-message-edit"));
		menu->addMenu (statusMenu);

		// set the menu as the parent QObject _owner_ of the statusMenu,
		// not as the QWidget visual parent
		static_cast<QObject*> (statusMenu)->setParent (menu);

		menu->addSeparator ();

		if (proto->GetFeatures () & IProtocol::PFMUCsJoinable)
			AddConferenceActions (menu, acc);

		menu->addAction (itm->GetIcon ("list-add-user"),
					AccountActions::tr ("Add contact..."),
					[acc] { AddAccountContact (acc); });

		if (const auto isnr = qobject_cast<ISupportNonRoster*> (accObj))
			menu->addAction (AccountActions::tr ("Chat with non-CL contact"),
					[isnr] { OpenNonRosterChat (isnr); });
		menu->addSeparator ();

		if (const auto ihsh = qobject_cast<IHaveServerHistory*> (accObj))
			AddServerHistoryActions (menu, ihsh);

		if (qobject_cast<IHaveMicroblogs*> (accObj))
		{
			menu->addAction (AccountActions::tr ("View microblogs..."),
					[acc] { GetProxyHolder ()->GetRootWindowsManager ()->AddTab (new MicroblogsTab { acc }); });
			menu->addSeparator ();
		}

		AddExtendedStatusIcons (menu, accObj);

		if (const auto& accActions = acc->GetActions ();
			!accActions.isEmpty ())
		{
			menu->addActions (accActions);
			for (auto action : accActions)
				action->setIcon (itm->GetIcon (action->property ("ActionIcon").toString ()));
			menu->addSeparator ();
		}

		if (qobject_cast<IHaveServiceDiscovery*> (accObj))
			menu->addAction (itm->GetIcon ("services"),
					AccountActions::tr ("Service discovery..."),
					[accObj]
					{
						auto w = new ServiceDiscoveryWidget ();
						w->SetAccount (accObj);
						GetProxyHolder ()->GetRootWindowsManager ()->AddTab (w);
					});
		if (qobject_cast<IHaveConsole*> (accObj))
			menu->addAction (itm->GetIcon ("utilities-terminal"),
					AccountActions::tr ("Console..."),
					[acc] { OpenConsole (acc); });

		menu->addSeparator ();

		const auto ipm = Core::Instance ().GetProxy ()->GetPluginsManager ();
		for (const auto plugin : ipm->GetAllCastableTo<IAccountActionsProvider*> ())
		{
			const auto& subs = plugin->CreateActions (acc);
			if (subs.isEmpty ())
				continue;

			for (const auto act : subs)
				act->setParent (menu);
			menu->addActions (subs);
			menu->addSeparator ();
		}

		if (const auto irma = qobject_cast<IRegManagedAccount*> (acc->GetQObject ());
			irma && irma->SupportsFeature (IRegManagedAccount::Feature::UpdatePass))
			menu->addAction (AccountActions::tr ("Update server password..."),
					[=] { UpdatePassword (acc, irma); })->setToolTip (AccountActions::tr ("Updates the account password on the server"));

		if (acc->GetAccountFeatures () & IAccount::FRenamable)
			menu->addAction (itm->GetIcon ("edit-rename"),
					AccountActions::tr ("Rename..."),
					[=] { RenameAccount (acc); });
		menu->addAction (AccountActions::tr ("Modify..."),
				[acc] { acc->OpenConfigurationDialog (); });
		menu->addAction (AccountActions::tr ("Remove"),
				[acc] { RemoveAccount (acc); });
	}
}
