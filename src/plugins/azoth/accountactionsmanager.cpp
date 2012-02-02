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
#include <util/util.h>
#include <interfaces/core/icoreproxy.h>
#include "interfaces/iaccount.h"
#include "interfaces/iprotocol.h"
#include "interfaces/isupportbookmarks.h"
#include "interfaces/isupportactivity.h"
#include "interfaces/isupportmood.h"
#include "interfaces/isupportgeolocation.h"
#include "interfaces/ihaveconsole.h"
#include "core.h"
#include "joinconferencedialog.h"
#include "bookmarksmanagerdialog.h"
#include "addcontactdialog.h"
#include "activitydialog.h"
#include "mooddialog.h"
#include "locationdialog.h"
#include "consolewidget.h"

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
	, AccountConsole_ (new QAction (tr ("Console..."), this))
	, AccountModify_ (new QAction (tr ("Modify..."), this))
	{
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
		connect (AccountConsole_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleAccountConsole ()));
		connect (AccountModify_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleAccountModify ()));
	}

	QList<QAction*> AccountActionsManager::GetMenuActions (QMenu *menu, QObject *accObj)
	{
		QList<QAction*> actions;

		QVariant objVar = QVariant::fromValue<QObject*> (accObj);

		IAccount *account = qobject_cast<IAccount*> (accObj);
		IProtocol *proto = qobject_cast<IProtocol*> (account->GetParentProtocol ());

		AccountJoinConference_->setEnabled (proto->GetFeatures () & IProtocol::PFMUCsJoinable);
		AccountJoinConference_->setProperty ("Azoth/AccountObject", objVar);
		actions << AccountJoinConference_;

		if (qobject_cast<ISupportBookmarks*> (objVar.value<QObject*> ()))
		{
			ISupportBookmarks *supBms =
					qobject_cast<ISupportBookmarks*> (objVar.value<QObject*> ());
			QVariantList bms = supBms->GetBookmarkedMUCs ();
			if (!bms.isEmpty ())
			{
				QMenu *bmsMenu = new QMenu (tr ("Join bookmarked conference"), menu);
				actions << bmsMenu->menuAction ();

				Q_FOREACH (const QObject *mucObj,
						qobject_cast<IAccount*> (objVar.value<QObject*> ())->GetCLEntries ())
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
					act->setProperty ("Azoth/AccountObject", objVar);
					act->setProperty ("Azoth/BMData", bm);
					connect (act,
							SIGNAL (triggered ()),
							this,
							SLOT (joinAccountConfFromBM ()));
				}
			}

			actions << AccountManageBookmarks_;
			AccountManageBookmarks_->setProperty ("Azoth/AccountObject", objVar);
		}
		actions << Util::CreateSeparator (menu);

		AccountAddContact_->setProperty ("Azoth/AccountObject", objVar);
		actions << AccountAddContact_;
		actions << Util::CreateSeparator (menu);

		if (qobject_cast<ISupportActivity*> (objVar.value<QObject*> ()))
		{
			AccountSetActivity_->setProperty ("Azoth/AccountObject", objVar);
			actions << AccountSetActivity_;
		}
		if (qobject_cast<ISupportMood*> (objVar.value<QObject*> ()))
		{
			AccountSetMood_->setProperty ("Azoth/AccountObject", objVar);
			actions << AccountSetMood_;
		}
		if (qobject_cast<ISupportGeolocation*> (objVar.value<QObject*> ()))
		{
			AccountSetLocation_->setProperty ("Azoth/AccountObject", objVar);
			actions << AccountSetLocation_;
		}
		actions << Util::CreateSeparator (menu);

		const auto& accActions = account->GetActions ();
		if (!accActions.isEmpty ())
		{
			actions += accActions;
			actions << Util::CreateSeparator (menu);
		}

		if (qobject_cast<IHaveConsole*> (objVar.value<QObject*> ()))
		{
			AccountConsole_->setProperty ("Azoth/AccountObject", objVar);
			actions << AccountConsole_;
		}
		actions << Util::CreateSeparator (menu);

		AccountModify_->setProperty ("Azoth/AccountObject", objVar);
		actions << AccountModify_;

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