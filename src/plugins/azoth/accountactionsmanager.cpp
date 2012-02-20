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

#include "accountactionsmanager.h"
#include <QAction>
#include <QMenu>
#include <QInputDialog>
#include <util/util.h>
#include <interfaces/core/icoreproxy.h>
#include "interfaces/iaccount.h"
#include "interfaces/imucjoinwidget.h"
#include "interfaces/iprotocol.h"
#include "interfaces/isupportbookmarks.h"
#include "interfaces/isupportactivity.h"
#include "interfaces/isupportmood.h"
#include "interfaces/isupportgeolocation.h"
#include "interfaces/ihaveservicediscovery.h"
#include "interfaces/ihaveconsole.h"
#include "core.h"
#include "joinconferencedialog.h"
#include "bookmarksmanagerdialog.h"
#include "addcontactdialog.h"
#include "activitydialog.h"
#include "mooddialog.h"
#include "locationdialog.h"
#include "consolewidget.h"
#include "servicediscoverywidget.h"

namespace LeechCraft
{
namespace Azoth
{
	AccountActionsManager::AccountActionsManager (QWidget *mw, QObject *parent)
	: QObject (parent)
	, MW_ (mw)
	, AccountJoinConference_ (new QAction (tr ("Join conference..."), this))
	, AccountManageBookmarks_ (new QAction (tr ("Manage bookmarks..."), this))
	, AccountAddContact_ (new QAction (tr ("Add contact..."), this))
	, AccountSetActivity_ (new QAction (tr ("Set activity..."), this))
	, AccountSetMood_ (new QAction (tr ("Set mood..."), this))
	, AccountSetLocation_ (new QAction (tr ("Set location..."), this))
	, AccountSD_ (new QAction (tr ("Service discovery..."), this))
	, AccountConsole_ (new QAction (tr ("Console..."), this))
	, AccountRename_ (new QAction (tr ("Rename..."), this))
	, AccountModify_ (new QAction (tr ("Modify..."), this))
	{
		AccountJoinConference_->setProperty ("ActionIcon", "irc-join-channel");
		AccountManageBookmarks_->setProperty ("ActionIcon", "bookmarks-organize");
		AccountAddContact_->setProperty ("ActionIcon", "list-add-user");
		AccountSetMood_->setProperty ("ActionIcon", "face-smile");
		AccountSD_->setProperty ("ActionIcon", "services");
		AccountConsole_->setProperty ("ActionIcon", "utilities-terminal");
		AccountRename_->setProperty ("ActionIcon", "edit-rename");
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
		connect (AccountRename_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleAccountRename ()));
		connect (AccountModify_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleAccountModify ()));
	}

	QList<QAction*> AccountActionsManager::GetMenuActions (QMenu *menu, QObject *accObj)
	{
		QList<QAction*> actions;

		IAccount *account = qobject_cast<IAccount*> (accObj);
		IProtocol *proto = qobject_cast<IProtocol*> (account->GetParentProtocol ());

		AccountJoinConference_->setEnabled (proto->GetFeatures () & IProtocol::PFMUCsJoinable);
		actions << AccountJoinConference_;

		if (qobject_cast<ISupportBookmarks*> (accObj))
		{
			auto supBms = qobject_cast<ISupportBookmarks*> (accObj);
			QVariantList bms = supBms->GetBookmarkedMUCs ();
			if (!bms.isEmpty ())
			{
				QMenu *bmsMenu = new QMenu (tr ("Join bookmarked conference"), menu);
				actions << bmsMenu->menuAction ();

				Q_FOREACH (const QObject *mucObj,
						qobject_cast<IAccount*> (accObj)->GetCLEntries ())
				{
					IMUCEntry *muc = qobject_cast<IMUCEntry*> (mucObj);
					if (!muc)
						continue;

					bms.removeAll (muc->GetIdentifyingData ());
				}

				Q_FOREACH (const QVariant& bm, bms)
				{
					const QVariantMap& map = bm.toMap ();

					QAction *act = bmsMenu->addAction (map ["HumanReadableName"].toString ());
					act->setProperty ("Azoth/BMData", bm);
					act->setProperty ("Azoth/AccountObject",
							QVariant::fromValue<QObject*> (accObj));
					connect (act,
							SIGNAL (triggered ()),
							this,
							SLOT (joinAccountConfFromBM ()));
				}
			}

			actions << AccountManageBookmarks_;
		}
		actions << Util::CreateSeparator (menu);

		actions << AccountAddContact_;
		actions << Util::CreateSeparator (menu);

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
			actions << Util::CreateSeparator (menu);
		}

		if (qobject_cast<IHaveServiceDiscovery*> (accObj))
			actions << AccountSD_;
		if (qobject_cast<IHaveConsole*> (accObj))
			actions << AccountConsole_;

		actions << Util::CreateSeparator (menu);

		if (account->GetAccountFeatures () & IAccount::FRenamable)
			actions << AccountRename_;
		actions << AccountModify_;

		Q_FOREACH (QAction *act, actions)
			act->setProperty ("Azoth/AccountObject",
					QVariant::fromValue<QObject*> (accObj));

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

	void AccountActionsManager::joinAccountConference ()
	{
		IAccount *account = GetAccountFromSender (sender (), Q_FUNC_INFO);
		if (!account)
			return;

		QList<IAccount*> accounts;
		accounts << account;
		auto dia = new JoinConferenceDialog (accounts, MW_);
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

		IProtocol *proto = qobject_cast<IProtocol*> (account->GetParentProtocol ());
		IMUCJoinWidget *imjw = qobject_cast<IMUCJoinWidget*> (proto->GetMUCJoinWidget ());
		imjw->SetIdentifyingData (bmData.toMap ());
		imjw->Join (account->GetObject ());
	}

	void AccountActionsManager::manageAccountBookmarks ()
	{
		IAccount *account = GetAccountFromSender (sender (), Q_FUNC_INFO);
		if (!account)
			return;

		auto dia = new BookmarksManagerDialog (MW_);
		dia->FocusOn (account);
		dia->show ();
		dia->setAttribute (Qt::WA_DeleteOnClose, true);
	}

	void AccountActionsManager::addAccountContact ()
	{
		IAccount *account = GetAccountFromSender (sender (), Q_FUNC_INFO);
		if (!account)
			return;

		AddContactDialog dia (account, MW_);
		if (dia.exec () != QDialog::Accepted)
			return;

		dia.GetSelectedAccount ()->RequestAuth (dia.GetContactID (),
				dia.GetReason (), dia.GetNick (), dia.GetGroups ());
	}

	void AccountActionsManager::handleAccountSetActivity ()
	{
		IAccount *account = GetAccountFromSender (sender (), Q_FUNC_INFO);
		if (!account)
			return;

		QObject *obj = sender ()->property ("Azoth/AccountObject").value<QObject*> ();
		ISupportActivity *activity = qobject_cast<ISupportActivity*> (obj);
		if (!activity)
		{
			qWarning () << Q_FUNC_INFO
					<< obj
					<< "doesn't support activity";
			return;
		}

		ActivityDialog dia (MW_);
		if (dia.exec () != QDialog::Accepted)
			return;

		activity->SetActivity (dia.GetGeneral (), dia.GetSpecific (), dia.GetText ());
	}

	void AccountActionsManager::handleAccountSetMood ()
	{
		IAccount *account = GetAccountFromSender (sender (), Q_FUNC_INFO);
		if (!account)
			return;

		QObject *obj = sender ()->property ("Azoth/AccountObject").value<QObject*> ();
		ISupportMood *mood = qobject_cast<ISupportMood*> (obj);
		if (!mood)
		{
			qWarning () << Q_FUNC_INFO
					<< obj
					<< "doesn't support mood";
			return;
		}

		MoodDialog dia (MW_);
		if (dia.exec () != QDialog::Accepted)
			return;

		mood->SetMood (dia.GetMood (), dia.GetText ());
	}

	void AccountActionsManager::handleAccountSetLocation ()
	{
		IAccount *account = GetAccountFromSender (sender (), Q_FUNC_INFO);
		if (!account)
			return;

		QObject *obj = account->GetObject ();
		ISupportGeolocation *loc = qobject_cast<ISupportGeolocation*> (obj);
		if (!loc)
		{
			qWarning () << Q_FUNC_INFO
					<< obj
					<< "doesn't support geolocation";
			return;
		}

		LocationDialog dia (MW_);
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
		w->SetAccount (account->GetObject ());
		emit gotSDWidget (w);
	}

	void AccountActionsManager::handleAccountConsole ()
	{
		IAccount *account = GetAccountFromSender (sender (), Q_FUNC_INFO);
		if (!account)
			return;

		if (!Account2CW_.contains (account))
		{
			ConsoleWidget *cw = new ConsoleWidget (account->GetObject ());
			Account2CW_ [account] = cw;
			connect (cw,
					SIGNAL (removeTab (QWidget*)),
					this,
					SLOT (consoleRemoved (QWidget*)));
		}

		emit gotConsoleWidget (Account2CW_ [account]);
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

	void AccountActionsManager::consoleRemoved (QWidget *w)
	{
		ConsoleWidget *cw = qobject_cast<ConsoleWidget*> (w);
		Account2CW_.remove (Account2CW_.key (cw));
	}
}
}