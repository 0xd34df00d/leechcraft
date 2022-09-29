/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "accountactionsmanager.h"
#include <QAction>
#include <QMenu>
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

namespace LC
{
namespace Azoth
{
	AccountActionsManager::AccountActionsManager (QObject *parent)
	: QObject (parent)
	, MenuChangeStatus_ (StatusChange::CreateMenu (this, std::bind_front (&AccountActionsManager::ChangeStatus, this)))
	, AccountJoinConference_ (new QAction (tr ("Join conference..."), this))
	, AccountManageBookmarks_ (new QAction (tr ("Manage bookmarks..."), this))
	, AccountAddContact_ (new QAction (tr ("Add contact..."), this))
	, AccountOpenNonRosterChat_ (new QAction (tr ("Chat with non-CL contact"), this))
	, AccountOpenServerHistory_ (new QAction (tr ("Open server history..."), this))
	, AccountConfigServerHistory_ (new QAction (tr ("Configure server history..."), this))
	, AccountViewMicroblogs_ (new QAction (tr ("View microblogs..."), this))
	, AccountSetActivity_ (new QAction (tr ("Set activity..."), this))
	, AccountSetMood_ (new QAction (tr ("Set mood..."), this))
	, AccountSetLocation_ (new QAction (tr ("Set location..."), this))
	, AccountSD_ (new QAction (tr ("Service discovery..."), this))
	, AccountConsole_ (new QAction (tr ("Console..."), this))
	, AccountUpdatePassword_ (new QAction (tr ("Update server password..."), this))
	, AccountRename_ (new QAction (tr ("Rename..."), this))
	, AccountModify_ (new QAction (tr ("Modify..."), this))
	, AccountRemove_ (new QAction (tr ("Remove"), this))
	{
		AccountJoinConference_->setProperty ("ActionIcon", "irc-join-channel");
		AccountManageBookmarks_->setProperty ("ActionIcon", "bookmarks-organize");
		AccountAddContact_->setProperty ("ActionIcon", "list-add-user");
		AccountOpenServerHistory_->setProperty ("ActionIcon", "network-server-database");
		AccountSetMood_->setProperty ("ActionIcon", "face-smile");
		AccountSD_->setProperty ("ActionIcon", "services");
		AccountConsole_->setProperty ("ActionIcon", "utilities-terminal");
		AccountUpdatePassword_->setToolTip (tr ("Updates the account's password on the server"));
		AccountRename_->setProperty ("ActionIcon", "edit-rename");
		MenuChangeStatus_->menuAction ()->setProperty ("ActionIcon", "im-status-message-edit");

		connect (AccountJoinConference_,
				SIGNAL (triggered ()),
				this,
				SLOT (joinAccountConference ()));
		connect (AccountManageBookmarks_,
				SIGNAL (triggered ()),
				this,
				SLOT (manageAccountBookmarks ()));
		connect (AccountAddContact_,
				SIGNAL (triggered ()),
				this,
				SLOT (addAccountContact ()));
		connect (AccountOpenNonRosterChat_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleOpenNonRoster ()));
		connect (AccountOpenServerHistory_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleOpenServerHistory ()));
		connect (AccountConfigServerHistory_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleConfigServerHistory ()));
		connect (AccountViewMicroblogs_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleAccountMicroblogs ()));
		connect (AccountSetActivity_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleAccountSetActivity ()));
		connect (AccountSetMood_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleAccountSetMood ()));
		connect (AccountSetLocation_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleAccountSetLocation ()));
		connect (AccountSD_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleAccountSD()));
		connect (AccountConsole_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleAccountConsole ()));
		connect (AccountUpdatePassword_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleUpdatePassword ()));
		connect (AccountRename_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleAccountRename ()));
		connect (AccountModify_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleAccountModify ()));
		connect (AccountRemove_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleAccountRemove ()));
	}

	QList<QAction*> AccountActionsManager::GetMenuActions (QMenu *menu, IAccount *account)
	{
		QList<QAction*> actions;

		const auto accObj = account->GetQObject ();
		IProtocol *proto = qobject_cast<IProtocol*> (account->GetParentProtocol ());

		actions << MenuChangeStatus_->menuAction ();
		actions << Util::CreateSeparator (menu);

		AccountJoinConference_->setEnabled (proto->GetFeatures () & IProtocol::PFMUCsJoinable);
		actions << AccountJoinConference_;
		actions << AddBMActions (menu, accObj);
		actions << AccountManageBookmarks_;
		actions << Util::CreateSeparator (menu);

		actions << AccountAddContact_;
		if (qobject_cast<ISupportNonRoster*> (accObj))
			actions << AccountOpenNonRosterChat_;
		actions << Util::CreateSeparator (menu);

		if (const auto ihsh = qobject_cast<IHaveServerHistory*> (accObj))
		{
			if (ihsh->HasFeature (ServerHistoryFeature::AccountSupportsHistory))
				actions << AccountOpenServerHistory_;

			if (ihsh->HasFeature (ServerHistoryFeature::Configurable))
				actions << AccountConfigServerHistory_;

			actions << Util::CreateSeparator (menu);
		}

		if (qobject_cast<IHaveMicroblogs*> (accObj))
		{
			actions << AccountViewMicroblogs_;
			actions << Util::CreateSeparator (menu);
		}

		if (qobject_cast<ISupportActivity*> (accObj))
			actions << AccountSetActivity_;
		if (qobject_cast<ISupportMood*> (accObj))
			actions << AccountSetMood_;
		if (qobject_cast<ISupportGeolocation*> (accObj))
			actions << AccountSetLocation_;
		actions << Util::CreateSeparator (menu);

		const auto& accActions = account->GetActions ();
		if (!accActions.isEmpty ())
		{
			actions += accActions;
			auto mgr = Core::Instance ().GetProxy ()->GetIconThemeManager ();
			for (auto action : actions)
				action->setIcon (mgr->GetIcon (action->property ("ActionIcon").toString ()));
			actions << Util::CreateSeparator (menu);
		}

		if (qobject_cast<IHaveServiceDiscovery*> (accObj))
			actions << AccountSD_;
		if (qobject_cast<IHaveConsole*> (accObj))
			actions << AccountConsole_;

		actions << Util::CreateSeparator (menu);

		const auto ipm = Core::Instance ().GetProxy ()->GetPluginsManager ();
		for (const auto plugin : ipm->GetAllCastableTo<IAccountActionsProvider*> ())
		{
			const auto& subs = plugin->CreateActions (account);
			if (subs.isEmpty ())
				continue;

			for (const auto act : subs)
				act->setParent (menu);

			actions += subs;
			actions << Util::CreateSeparator (menu);
		}

		if (auto managed = qobject_cast<IRegManagedAccount*> (account->GetQObject ()))
			if (managed->SupportsFeature (IRegManagedAccount::Feature::UpdatePass))
				actions << AccountUpdatePassword_;

		if (account->GetAccountFeatures () & IAccount::FRenamable)
			actions << AccountRename_;
		actions << AccountModify_;
		actions << AccountRemove_;

		const auto& accObjVar = QVariant::fromValue<QObject*> (accObj);
		std::function<void (QList<QAction*>)> actionSetter = [&actionSetter, &accObjVar] (const QList<QAction*>& actions)
		{
			for (auto act : actions)
			{
				act->setProperty ("Azoth/AccountObject", accObjVar);

				if (act->menu ())
					actionSetter (act->menu ()->actions ());
			}
		};
		actionSetter (actions);

		return actions;
	}

	QList<QAction*> AccountActionsManager::AddBMActions (QMenu *menu, QObject *accObj)
	{
		if (!qobject_cast<ISupportBookmarks*> (accObj))
			return {};

		auto supBms = qobject_cast<ISupportBookmarks*> (accObj);
		auto bms = supBms->GetBookmarkedMUCs ();
		if (bms.isEmpty ())
			return {};

		QList<QAction*> actions;

		QMenu *bmsMenu = new QMenu (tr ("Join bookmarked conference"), menu);
		actions << bmsMenu->menuAction ();

		for (auto mucObj : qobject_cast<IAccount*> (accObj)->GetCLEntries ())
		{
			IMUCEntry *muc = qobject_cast<IMUCEntry*> (mucObj);
			if (!muc)
				continue;

			bms.removeAll (muc->GetIdentifyingData ());
		}

		for (const auto& bm : bms)
		{
			const QVariantMap& map = bm.toMap ();

			auto name = map ["StoredName"].toString ();
			const auto& hrName = map ["HumanReadableName"].toString ();
			if (name.isEmpty ())
				name = hrName;
			QAction *act = bmsMenu->addAction (name);
			act->setProperty ("Azoth/BMData", bm);
			act->setToolTip (hrName);
			connect (act,
					SIGNAL (triggered ()),
					this,
					SLOT (joinAccountConfFromBM ()));
		}

		return actions;
	}

	namespace
	{
		IAccount* GetAccountFromSender (QObject *sender, const char *func)
		{
			if (!sender)
			{
				qWarning () << func
						<< "no sender";
				return 0;
			}

			const QVariant& objVar = sender->property ("Azoth/AccountObject");
			QObject *object = objVar.value<QObject*> ();
			if (!object)
			{
				qWarning () << func
						<< "no object in Azoth/AccountObject property of the sender"
						<< sender
						<< objVar;
				return 0;
			}

			IAccount *account = qobject_cast<IAccount*> (object);
			if (!account)
				qWarning () << func
						<< "object"
						<< object
						<< "could not be cast to IAccount";

			return account;
		}
	}

	void AccountActionsManager::ChangeStatus (State state, const QString& text)
	{
		const auto acc = GetAccountFromSender (sender (), Q_FUNC_INFO);

		EntryStatus status;
		if (state != SInvalid)
			status = EntryStatus { state, StatusChange::GetStatusText (state, text) };
		else
		{
			SetStatusDialog ssd { acc->GetAccountID () };
			if (ssd.exec () != QDialog::Accepted)
				return;

			status = EntryStatus { ssd.GetState (), ssd.GetStatusText () };
		}

		acc->ChangeState (status);
	}

	namespace
	{
		QWidget* GetDialogParentWidget ()
		{
			return GetProxyHolder ()->GetRootWindowsManager ()->GetPreferredWindow ();
		}
	}

	void AccountActionsManager::joinAccountConference ()
	{
		IAccount *account = GetAccountFromSender (sender (), Q_FUNC_INFO);
		if (!account)
			return;

		QList<IAccount*> accounts;
		accounts << account;
		auto dia = new JoinConferenceDialog (accounts, GetDialogParentWidget ());
		dia->show ();
		dia->setAttribute (Qt::WA_DeleteOnClose, true);
	}

	void AccountActionsManager::joinAccountConfFromBM ()
	{
		IAccount *account = GetAccountFromSender (sender (), Q_FUNC_INFO);
		if (!account)
			return;

		const QVariant& bmData = sender ()->property ("Azoth/BMData");
		if (bmData.isNull ())
			return;

		const auto proto = qobject_cast<IMUCProtocol*> (account->GetParentProtocol ());
		if (!proto)
		{
			qWarning () << Q_FUNC_INFO
					<< account->GetAccountName ()
					<< "parent protocol does not implement IMUCProtocol";
			return;
		}

		auto jWidget = proto->GetMUCJoinWidget ();
		IMUCJoinWidget *imjw = qobject_cast<IMUCJoinWidget*> (jWidget);
		imjw->SetIdentifyingData (bmData.toMap ());
		imjw->Join (account->GetQObject ());

		jWidget->deleteLater ();
	}

	void AccountActionsManager::manageAccountBookmarks ()
	{
		IAccount *account = GetAccountFromSender (sender (), Q_FUNC_INFO);
		if (!account)
			return;

		auto dia = new BookmarksManagerDialog (GetDialogParentWidget ());
		dia->FocusOn (account);
		dia->show ();
	}

	void AccountActionsManager::addAccountContact ()
	{
		IAccount *account = GetAccountFromSender (sender (), Q_FUNC_INFO);
		if (!account)
			return;

		AddContactDialog dia (account, GetDialogParentWidget ());
		if (dia.exec () != QDialog::Accepted)
			return;

		dia.GetSelectedAccount ()->RequestAuth (dia.GetContactID (),
				dia.GetReason (), dia.GetNick (), dia.GetGroups ());
	}

	void AccountActionsManager::handleOpenNonRoster ()
	{
		const auto obj = sender ()->property ("Azoth/AccountObject").value<QObject*> ();
		const auto isnr = qobject_cast<ISupportNonRoster*> (obj);
		if (!isnr)
			return;

		QObject *entryObj = nullptr;
		QString contactId;
		while (!entryObj)
		{
			contactId = QInputDialog::getText (nullptr,
					tr ("Open chat with non-roster contact"),
					tr ("Enter ID of the contact you wish to open chat with:"),
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
						tr ("Error opening chat: %1")
							.arg (QString::fromUtf8 (e.what ())));
			}
		}

		const auto entry = qobject_cast<ICLEntry*> (entryObj);
		Core::Instance ().GetChatTabsManager ()->OpenChat (entry, true);
	}

	void AccountActionsManager::handleOpenServerHistory ()
	{
		const auto obj = sender ()->property ("Azoth/AccountObject").value<QObject*> ();
		const auto ihsh = qobject_cast<IHaveServerHistory*> (obj);
		if (!ihsh)
			return;

		GetProxyHolder ()->GetRootWindowsManager ()->AddTab (new ServerHistoryWidget { ihsh });
	}

	void AccountActionsManager::handleConfigServerHistory ()
	{
		const auto obj = sender ()->property ("Azoth/AccountObject").value<QObject*> ();
		const auto ihsh = qobject_cast<IHaveServerHistory*> (obj);
		ihsh->OpenServerHistoryConfiguration ();
	}

	void AccountActionsManager::handleAccountMicroblogs ()
	{
		IAccount *account = GetAccountFromSender (sender (), Q_FUNC_INFO);
		if (!account)
			return;

		GetProxyHolder ()->GetRootWindowsManager ()->AddTab (new MicroblogsTab { account });
	}

	namespace
	{
		template<typename R, typename C, typename F>
		void InitSelfDialog (IAccount *acc, R (C::*getter) (const QString&) const, F act)
		{
			const auto iesi = qobject_cast<IExtSelfInfoAccount*> (acc->GetQObject ());
			if (!iesi)
				return;

			const auto selfObj = iesi->GetSelfContact ();
			if (!selfObj)
				return;

			const auto iface = qobject_cast<C*> (selfObj);
			if (!iface)
				return;

			act ((iface->*getter) ({}));
		}
	}

	void AccountActionsManager::handleAccountSetActivity ()
	{
		IAccount *account = GetAccountFromSender (sender (), Q_FUNC_INFO);
		if (!account)
			return;

		QObject *obj = account->GetQObject ();
		ISupportActivity *activity = qobject_cast<ISupportActivity*> (obj);
		if (!activity)
		{
			qWarning () << Q_FUNC_INFO
					<< obj
					<< "doesn't support activity";
			return;
		}

		ActivityDialog dia (GetDialogParentWidget ());

		InitSelfDialog (account, &IHaveContactActivity::GetUserActivity,
				[&dia] (const ActivityInfo& info) { dia.SetActivityInfo (info); });

		if (dia.exec () != QDialog::Accepted)
			return;

		activity->SetActivity (dia.GetActivityInfo ());
	}

	void AccountActionsManager::handleAccountSetMood ()
	{
		IAccount *account = GetAccountFromSender (sender (), Q_FUNC_INFO);
		if (!account)
			return;

		QObject *obj = account->GetQObject ();
		ISupportMood *mood = qobject_cast<ISupportMood*> (obj);
		if (!mood)
		{
			qWarning () << Q_FUNC_INFO
					<< obj
					<< "doesn't support mood";
			return;
		}

		MoodDialog dia (GetDialogParentWidget ());

		InitSelfDialog (account, &IHaveContactMood::GetUserMood,
				[&dia] (const MoodInfo& info) { dia.SetMood (info); });

		if (dia.exec () != QDialog::Accepted)
			return;

		mood->SetMood (dia.GetMood ());
	}

	void AccountActionsManager::handleAccountSetLocation ()
	{
		IAccount *account = GetAccountFromSender (sender (), Q_FUNC_INFO);
		if (!account)
			return;

		QObject *obj = account->GetQObject ();
		ISupportGeolocation *loc = qobject_cast<ISupportGeolocation*> (obj);
		if (!loc)
		{
			qWarning () << Q_FUNC_INFO
					<< obj
					<< "doesn't support geolocation";
			return;
		}

		LocationDialog dia (GetDialogParentWidget ());
		if (dia.exec () != QDialog::Accepted)
			return;

		loc->SetGeolocationInfo (dia.GetInfo ());
	}

	void AccountActionsManager::handleAccountSD ()
	{
		IAccount *account = GetAccountFromSender (sender (), Q_FUNC_INFO);
		if (!account)
			return;

		auto w = new ServiceDiscoveryWidget ();
		w->SetAccount (account->GetQObject ());
		GetProxyHolder ()->GetRootWindowsManager ()->AddTab (w);
	}

	void AccountActionsManager::handleAccountConsole ()
	{
		IAccount *account = GetAccountFromSender (sender (), Q_FUNC_INFO);
		if (!account)
			return;

		auto& cw = Account2CW_ [account];
		if (!cw)
		{
			cw = new ConsoleWidget (account->GetQObject ());
			connect (cw,
					&ConsoleWidget::removeTab,
					this,
					[this, account] { Account2CW_.remove (account); });
		}

		GetProxyHolder ()->GetRootWindowsManager ()->AddTab (cw->GetTitle (), cw);
	}

	void AccountActionsManager::handleUpdatePassword ()
	{
		auto account = GetAccountFromSender (sender (), Q_FUNC_INFO);
		if (!account)
			return;

		const auto& name = account->GetAccountName ();
		const auto& pass = QInputDialog::getText (0,
				tr ("Change password"),
				tr ("Enter new password for account %1 (the password will be updated on server):")
					.arg (name),
				QLineEdit::Password);
		if (pass.isEmpty ())
			return;

		auto managed = qobject_cast<IRegManagedAccount*> (account->GetQObject ());
		managed->UpdateServerPassword (pass);
	}

	void AccountActionsManager::handleAccountRename ()
	{
		IAccount *account = GetAccountFromSender (sender (), Q_FUNC_INFO);
		if (!account)
			return;

		const QString& name = account->GetAccountName ();
		const QString& newName = QInputDialog::getText (0,
				tr ("Rename account"),
				tr ("Enter new name for account %1:")
					.arg (name),
				QLineEdit::Normal,
				name);
		if (newName.isEmpty ())
			return;

		account->RenameAccount (newName);
	}

	void AccountActionsManager::handleAccountModify ()
	{
		IAccount *account = GetAccountFromSender (sender (), Q_FUNC_INFO);
		if (!account)
			return;

		account->OpenConfigurationDialog ();
	}

	void AccountActionsManager::handleAccountRemove ()
	{
		RemoveAccount (GetAccountFromSender (sender (), Q_FUNC_INFO));
	}
}
}
