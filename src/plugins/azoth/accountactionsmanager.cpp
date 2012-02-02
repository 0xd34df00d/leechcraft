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
#include "core.h"
#include "joinconferencedialog.h"
#include "bookmarksmanagerdialog.h"

namespace LeechCraft
{
namespace Azoth
{
	AccountActionsManager::AccountActionsManager (QWidget *mw, QObject *parent)
	: QObject (parent)
	, MW_ (mw)
	, AccountJoinConference_ (new QAction (tr ("Join conference..."), this))
	, AccountManageBookmarks_ (new QAction (tr ("Manage bookmarks..."), this))
	{
		connect (AccountJoinConference_,
				SIGNAL (triggered ()),
				this,
				SLOT (joinAccountConference ()));
		connect (AccountManageBookmarks_,
				SIGNAL (triggered ()),
				this,
				SLOT (manageAccountBookmarks ()));
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

			actions << Util::CreateSeparator (menu);
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
}
}