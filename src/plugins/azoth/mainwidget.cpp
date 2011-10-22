/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#include "mainwidget.h"
#include <QMenu>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QToolButton>
#include <QInputDialog>
#include <QTimer>
#include <util/util.h>
#include <interfaces/core/icoreproxy.h>
#include "interfaces/iclentry.h"
#include "interfaces/ihaveconsole.h"
#include "interfaces/isupportactivity.h"
#include "interfaces/isupportmood.h"
#include "interfaces/isupportgeolocation.h"
#include "interfaces/isupportbookmarks.h"
#include "interfaces/imucjoinwidget.h"
#include "core.h"
#include "sortfilterproxymodel.h"
#include "setstatusdialog.h"
#include "contactlistdelegate.h"
#include "xmlsettingsmanager.h"
#include "addcontactdialog.h"
#include "joinconferencedialog.h"
#include "bookmarksmanagerdialog.h"
#include "chattabsmanager.h"
#include "consolewidget.h"
#include "activitydialog.h"
#include "mooddialog.h"
#include "locationdialog.h"
#include "util.h"

namespace LeechCraft
{
namespace Azoth
{
	MainWidget::MainWidget (QWidget *parent)
	: QWidget (parent)
	, MainMenu_ (new QMenu (tr ("Azoth menu")))
	, MenuButton_ (new QToolButton (this))
	, ProxyModel_ (new SortFilterProxyModel ())
	{
		MainMenu_->setIcon (QIcon (":/plugins/azoth/resources/images/azoth.svg"));

		Ui_.setupUi (this);
		Ui_.BottomLayout_->insertWidget (0, MenuButton_);
#if QT_VERSION >= 0x040700
		Ui_.FilterLine_->setPlaceholderText (tr ("Search..."));
#endif
		Ui_.CLTree_->setFocusProxy (Ui_.FilterLine_);

		Ui_.CLTree_->setItemDelegate (new ContactListDelegate (Ui_.CLTree_));
		ProxyModel_->setSourceModel (Core::Instance ().GetCLModel ());
		Ui_.CLTree_->setModel (ProxyModel_);

		Ui_.CLTree_->viewport ()->setAcceptDrops (true);

		connect (Core::Instance ().GetChatTabsManager (),
				SIGNAL (entryMadeCurrent (QObject*)),
				this,
				SLOT (handleEntryMadeCurrent (QObject*)),
				Qt::QueuedConnection);

#ifdef Q_WS_WIN32
		connect (Ui_.CLTree_,
				SIGNAL (clicked (const QModelIndex&)),
				this,
				SLOT (on_CLTree__activated (const QModelIndex&)));
#endif

		connect (Ui_.CLTree_,
				SIGNAL (activated (const QModelIndex&)),
				this,
				SLOT (clearFilter ()));

		connect (Ui_.FilterLine_,
				SIGNAL (textChanged (const QString&)),
				ProxyModel_,
				SLOT (setFilterFixedString (const QString&)));

		connect (ProxyModel_,
				SIGNAL (rowsInserted (const QModelIndex&, int, int)),
				this,
				SLOT (handleRowsInserted (const QModelIndex&, int, int)));
		connect (ProxyModel_,
				SIGNAL (rowsRemoved (const QModelIndex&, int, int)),
				this,
				SLOT (rebuildTreeExpansions ()));
		connect (ProxyModel_,
				SIGNAL (modelReset ()),
				this,
				SLOT (rebuildTreeExpansions ()));
		connect (ProxyModel_,
				SIGNAL (mucMode ()),
				Ui_.CLTree_,
				SLOT (expandAll ()));

		QMetaObject::invokeMethod (Ui_.CLTree_,
				"expandToDepth",
				Qt::QueuedConnection,
				Q_ARG (int, 0));

		if (Core::Instance ().GetCLModel ()->rowCount ())
			QMetaObject::invokeMethod (this,
					"handleRowsInserted",
					Qt::QueuedConnection,
					Q_ARG (QModelIndex, QModelIndex ()),
					Q_ARG (int, 0),
					Q_ARG (int, Core::Instance ().GetCLModel ()->rowCount () - 1));

		CreateMenu ();
		MenuButton_->setMenu (MainMenu_);
		MenuButton_->setIcon (MainMenu_->icon ());
		MenuButton_->setPopupMode (QToolButton::InstantPopup);

		MenuChangeStatus_ = CreateStatusChangeMenu (SLOT (handleChangeStatusRequested ()), true);
		TrayChangeStatus_ = CreateStatusChangeMenu (SLOT (handleChangeStatusRequested ()), true);

		Ui_.FastStatusButton_->setMenu (CreateStatusChangeMenu (SLOT (fastStateChangeRequested ())));
		Ui_.FastStatusButton_->setDefaultAction (new QAction (tr ("Set status"), this));
		UpdateFastStatusButton (SOnline);
		connect (Ui_.FastStatusButton_->defaultAction (),
				SIGNAL (triggered ()),
				this,
				SLOT (applyFastStatus ()));
		connect (Ui_.FastStatusText_,
				SIGNAL (returnPressed ()),
				this,
				SLOT (applyFastStatus ()));

		AccountJoinConference_ = new QAction (tr ("Join conference..."), this);
		connect (AccountJoinConference_,
				SIGNAL (triggered ()),
				this,
				SLOT (joinAccountConference ()));

		AccountManageBookmarks_ = new QAction (tr ("Manage bookmarks..."), this);
		connect (AccountManageBookmarks_,
				SIGNAL (triggered ()),
				this,
				SLOT (manageAccountBookmarks ()));

		AccountAddContact_ = new QAction (tr ("Add contact..."), this);
		connect (AccountAddContact_,
				SIGNAL (triggered ()),
				this,
				SLOT (addAccountContact ()));

		AccountSetActivity_ = new QAction (tr ("Set activity..."), this);
		connect (AccountSetActivity_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleAccountSetActivity ()));

		AccountSetMood_ = new QAction (tr ("Set mood..."), this);
		connect (AccountSetMood_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleAccountSetMood ()));

		AccountSetLocation_ = new QAction (tr ("Set location..."), this);
		connect (AccountSetLocation_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleAccountSetLocation ()));

		AccountConsole_ = new QAction (tr ("Console..."), this);
		connect (AccountConsole_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleAccountConsole ()));

		XmlSettingsManager::Instance ().RegisterObject ("ShowMenuBar",
				this, "menuBarVisibilityToggled");
		menuBarVisibilityToggled ();
	}

	QList<QAction*> MainWidget::GetMenuActions()
	{
		return QList<QAction*> () << MainMenu_->actions ();
	}

	QMenu* MainWidget::GetChangeStatusMenu () const
	{
		return TrayChangeStatus_;
	}

	void MainWidget::CreateMenu ()
	{
		MainMenu_->addSeparator ();

		MainMenu_->addAction (tr ("Add contact..."),
				this,
				SLOT (handleAddContactRequested ()));
		MainMenu_->addAction (tr ("Join conference..."),
				&Core::Instance (),
				SLOT (handleMucJoinRequested ()));

		MainMenu_->addSeparator ();

		MainMenu_->addAction (tr ("Manage bookmarks..."),
				this,
				SLOT (handleManageBookmarks ()));

		MainMenu_->addSeparator ();

		MainMenu_->addAction (tr ("Add account..."),
				this,
				SLOT (handleAddAccountRequested ()));

		MainMenu_->addSeparator ();

		QAction *showOffline = MainMenu_->addAction (tr ("Show offline contacts"));
		showOffline->setProperty ("ActionIcon", "azoth_showoffline");
		showOffline->setCheckable (true);
		bool show = XmlSettingsManager::Instance ()
				.Property ("ShowOfflineContacts", true).toBool ();
		ProxyModel_->showOfflineContacts (show);
		showOffline->setChecked (show);
		connect (showOffline,
				SIGNAL (toggled (bool)),
				this,
				SLOT (handleShowOffline (bool)));
	}

	QMenu* MainWidget::CreateStatusChangeMenu (const char *slot, bool withCustom)
	{
		QMenu *result = new QMenu (tr ("Change status"));
		result->addAction (Core::Instance ().GetIconForState (SOnline),
				tr ("Online"), this, slot)->
					setProperty ("Azoth/TargetState",
							QVariant::fromValue<State> (SOnline));
		result ->addAction (Core::Instance ().GetIconForState (SChat),
				tr ("Free to chat"), this, slot)->
					setProperty ("Azoth/TargetState",
							QVariant::fromValue<State> (SChat));
		result ->addAction (Core::Instance ().GetIconForState (SAway),
				tr ("Away"), this, slot)->
					setProperty ("Azoth/TargetState",
							QVariant::fromValue<State> (SAway));
		result ->addAction (Core::Instance ().GetIconForState (SDND),
				tr ("DND"), this, slot)->
					setProperty ("Azoth/TargetState",
							QVariant::fromValue<State> (SDND));
		result ->addAction (Core::Instance ().GetIconForState (SXA),
				tr ("Extended away"), this, slot)->
					setProperty ("Azoth/TargetState",
							QVariant::fromValue<State> (SXA));
		result ->addAction (Core::Instance ().GetIconForState (SOffline),
				tr ("Offline"), this, slot)->
					setProperty ("Azoth/TargetState",
							QVariant::fromValue<State> (SOffline));

		if (withCustom)
		{
			result->addSeparator ();
			result->addAction (tr ("Custom..."),
					this,
					SLOT (handleChangeStatusRequested ()));
		}
		return result;
	}

	void MainWidget::UpdateFastStatusButton (State state)
	{
		Ui_.FastStatusButton_->defaultAction ()->setIcon (Core::Instance ().GetIconForState (state));
		Ui_.FastStatusButton_->setProperty ("Azoth/TargetState",
				QVariant::fromValue<State> (state));
	}

	IAccount* MainWidget::GetAccountFromSender (const char *func)
	{
		if (!sender ())
		{
			qWarning () << func
					<< "no sender";
			return 0;
		}

		const QVariant& objVar = sender ()->property ("Azoth/AccountObject");
		QObject *object = objVar.value<QObject*> ();
		if (!object)
		{
			qWarning () << func
					<< "no object in Azoth/AccountObject property of the sender"
					<< sender ()
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

	void MainWidget::on_CLTree__activated (const QModelIndex& index)
	{
		if (index.data (Core::CLREntryType).value<Core::CLEntryType> () != Core::CLETContact)
			return;

		Core::Instance ().OpenChat (ProxyModel_->mapToSource (index));
	}

	void MainWidget::on_CLTree__customContextMenuRequested (const QPoint& pos)
	{
		const QModelIndex& index = Ui_.CLTree_->indexAt (pos);
		if (!index.isValid ())
			return;

		QMenu *menu = new QMenu (tr ("Entry context menu"));
		QList<QAction*> actions;
		switch (index.data (Core::CLREntryType).value<Core::CLEntryType> ())
		{
		case Core::CLETContact:
		{
			QObject *obj = index.data (Core::CLREntryObject).value<QObject*> ();
			ICLEntry *entry = qobject_cast<ICLEntry*> (obj);
			const QList<QAction*>& allActions = Core::Instance ().GetEntryActions (entry);
			Q_FOREACH (QAction *action, allActions)
				if (Core::Instance ().GetAreasForAction (action)
						.contains (Core::CLEAAContactListCtxtMenu) ||
					action->isSeparator ())
					actions << action;
			break;
		}
		case Core::CLETCategory:
		{
			QAction *rename = new QAction (tr ("Rename group..."), this);
			QVariant objVar = index.parent ().data (Core::CLRAccountObject);;
			rename->setProperty ("Azoth/OldGroupName", index.data ());
			rename->setProperty ("Azoth/AccountObject", objVar);
			connect (rename,
					SIGNAL (triggered ()),
					this,
					SLOT (handleCatRenameTriggered ()));
			actions << rename;
			break;
		}
		case Core::CLETAccount:
		{
			QVariant objVar = index.data (Core::CLRAccountObject);

			IAccount *account = qobject_cast<IAccount*> (objVar.value<QObject*> ());
			IProtocol *proto = qobject_cast<IProtocol*> (account->GetParentProtocol ());

			AccountJoinConference_->setEnabled (proto->GetFeatures () & IProtocol::PFMUCsJoinable);
			AccountJoinConference_->setProperty ("Azoth/AccountObject", objVar);

			AccountAddContact_->setProperty ("Azoth/AccountObject", objVar);
			actions << AccountJoinConference_;

			if (qobject_cast<ISupportBookmarks*> (objVar.value<QObject*> ()))
			{
				ISupportBookmarks *supBms =
						qobject_cast<ISupportBookmarks*> (objVar.value<QObject*> ());
				QVariantList bms = supBms->GetBookmarkedMUCs ();
				if (!bms.isEmpty ())
				{
					QMenu *bmsMenu = new QMenu (tr ("Join bookmarked conference"));
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

			actions << AccountAddContact_;
			actions << Util::CreateSeparator (menu);

			Q_FOREACH (QAction *act, MenuChangeStatus_->actions ())
			{
				if (act->isSeparator ())
					continue;

				act->setData (objVar);

				QVariant stateVar = act->property ("Azoth/TargetState");
				if (!stateVar.isNull ())
				{
					State state = stateVar.value<State> ();
					act->setIcon (Core::Instance ().GetIconForState (state));
				}
			}
			actions << MenuChangeStatus_->menuAction ();

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

			QList<QAction*> accActions = account->GetActions ();
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
			break;
		}
		default:
			break;
		}
		if (!actions.size ())
		{
			delete menu;
			return;
		}

		menu->addActions (actions);
		menu->exec (Ui_.CLTree_->mapToGlobal (pos));
	}

	void MainWidget::handleChangeStatusRequested ()
	{
		QAction *action = qobject_cast<QAction*> (sender ());
		if (!action)
		{
			qWarning () << Q_FUNC_INFO
					<< sender ()
					<< "is not an action";
			return;
		}

		QObject *obj = action->data ().value<QObject*> ();
		IAccount *acc = qobject_cast<IAccount*> (obj);
		if (obj && !acc)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to cast"
					<< obj
					<< "to IAccount";
			return;
		}

		QVariant stateVar = action->property ("Azoth/TargetState");
		EntryStatus status;
		if (!stateVar.isNull ())
		{
			State state = stateVar.value<State> ();
			const QString& propName = "DefaultStatus" + QString::number (state);
			const QString& text = XmlSettingsManager::Instance ()
					.property (propName.toLatin1 ()).toString ();
			status = EntryStatus (state, text);
		}
		else
		{
			SetStatusDialog *ssd = new SetStatusDialog (this);
			if (ssd->exec () != QDialog::Accepted)
				return;

			status = EntryStatus (ssd->GetState (), ssd->GetStatusText ());
		}

		if (acc)
			acc->ChangeState (status);
		else
			Q_FOREACH (IAccount *acc, Core::Instance ().GetAccounts ())
				acc->ChangeState (status);
	}

	void MainWidget::fastStateChangeRequested ()
	{
		UpdateFastStatusButton (sender ()->
					property ("Azoth/TargetState").value<State> ());
		applyFastStatus ();
	}

	void MainWidget::applyFastStatus ()
	{
		const QString& text = Ui_.FastStatusText_->text ();
		Ui_.FastStatusText_->setText (QString ());
		State state = Ui_.FastStatusButton_->
				property ("Azoth/TargetState").value<State> ();

		EntryStatus status (state, text);
		Q_FOREACH (IAccount *acc, Core::Instance ().GetAccounts ())
			acc->ChangeState (status);
	}

	void MainWidget::handleCatRenameTriggered ()
	{
		if (!sender ())
		{
			qWarning () << Q_FUNC_INFO
					<< "null sender()";
			return;
		}
		sender ()->deleteLater ();

		const QString& group = sender ()->property ("Azoth/OldGroupName").toString ();

		const QString& newGroup = QInputDialog::getText (this,
				tr ("Rename group"),
				tr ("Enter new group name for %1:")
					.arg (group),
				QLineEdit::Normal,
				group);
		if (newGroup.isEmpty () || newGroup == group)
			return;

		QObject *accObj = sender ()->property ("Azoth/AccountObject").value<QObject*> ();
		IAccount *acc = qobject_cast<IAccount*> (accObj);
		if (!acc)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to cast"
					<< accObj
					<< "to IAccount";
			return;
		}

		Q_FOREACH (QObject *entryObj, acc->GetCLEntries ())
		{
			ICLEntry *entry = qobject_cast<ICLEntry*> (entryObj);
			if (!entry)
			{
				qWarning () << Q_FUNC_INFO
						<< "unable to cast"
						<< entryObj
						<< "to ICLEntry";
				continue;
			}

			QStringList groups = entry->Groups ();
			if (groups.removeAll (group))
			{
				groups << newGroup;
				entry->SetGroups (groups);
			}
		}
	}

	void MainWidget::joinAccountConference ()
	{
		IAccount *account = GetAccountFromSender (Q_FUNC_INFO);
		if (!account)
			return;

		QList<IAccount*> accounts;
		accounts << account;
		JoinConferenceDialog *dia = new JoinConferenceDialog (accounts,
				Core::Instance ().GetProxy ()->GetMainWindow ());
		dia->show ();
		dia->setAttribute (Qt::WA_DeleteOnClose, true);
	}

	void MainWidget::joinAccountConfFromBM ()
	{
		IAccount *account = GetAccountFromSender (Q_FUNC_INFO);
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

	void MainWidget::manageAccountBookmarks ()
	{
		IAccount *account = GetAccountFromSender (Q_FUNC_INFO);
		if (!account)
			return;

		BookmarksManagerDialog *dia =
				new BookmarksManagerDialog (Core::Instance ()
						.GetProxy ()->GetMainWindow ());
		dia->FocusOn (account);
		dia->show ();
		dia->setAttribute (Qt::WA_DeleteOnClose, true);
	}

	void MainWidget::addAccountContact ()
	{
		IAccount *account = GetAccountFromSender (Q_FUNC_INFO);
		if (!account)
			return;

		std::auto_ptr<AddContactDialog> dia (new AddContactDialog (account, this));
		if (dia->exec () != QDialog::Accepted)
			return;

		dia->GetSelectedAccount ()->RequestAuth (dia->GetContactID (),
					dia->GetReason (),
					dia->GetNick (),
					dia->GetGroups ());
	}

	void MainWidget::handleAccountSetActivity ()
	{
		IAccount *account = GetAccountFromSender (Q_FUNC_INFO);
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

		std::auto_ptr<ActivityDialog> dia (new ActivityDialog (this));
		if (dia->exec () != QDialog::Accepted)
			return;

		activity->SetActivity (dia->GetGeneral (), dia->GetSpecific (), dia->GetText ());
	}

	void MainWidget::handleAccountSetMood ()
	{
		IAccount *account = GetAccountFromSender (Q_FUNC_INFO);
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

		std::auto_ptr<MoodDialog> dia (new MoodDialog (this));
		if (dia->exec () != QDialog::Accepted)
			return;

		mood->SetMood (dia->GetMood (), dia->GetText ());
	}

	void MainWidget::handleAccountSetLocation ()
	{
		IAccount *account = GetAccountFromSender (Q_FUNC_INFO);
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

		std::auto_ptr<LocationDialog> dia (new LocationDialog (this));
		if (dia->exec () != QDialog::Accepted)
			return;

		loc->SetGeolocationInfo (dia->GetInfo ());
	}

	void MainWidget::handleAccountConsole ()
	{
		IAccount *account = GetAccountFromSender (Q_FUNC_INFO);
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

	void MainWidget::handleManageBookmarks ()
	{
		BookmarksManagerDialog *dia = new BookmarksManagerDialog (this);
		dia->setAttribute (Qt::WA_DeleteOnClose, true);
		dia->show ();
	}

	void MainWidget::handleAddAccountRequested ()
	{
		InitiateAccountAddition (this);
	}

	void MainWidget::handleAddContactRequested ()
	{
		std::auto_ptr<AddContactDialog> dia (new AddContactDialog (0, this));
		if (dia->exec () != QDialog::Accepted)
			return;

		if (!dia->GetSelectedAccount ())
			return;

		dia->GetSelectedAccount ()->RequestAuth (dia->GetContactID (),
					dia->GetReason (),
					dia->GetNick (),
					dia->GetGroups ());
	}

	void MainWidget::handleShowOffline (bool show)
	{
		XmlSettingsManager::Instance ().setProperty ("ShowOfflineContacts", show);
		ProxyModel_->showOfflineContacts (show);
	}

	void MainWidget::clearFilter ()
	{
		if (!Ui_.FilterLine_->text ().isEmpty ())
			Ui_.FilterLine_->setText (QString ());
	}

	void MainWidget::handleEntryMadeCurrent (QObject *obj)
	{
		const bool isMUC = qobject_cast<IMUCEntry*> (obj);

		if (XmlSettingsManager::Instance ().property ("AutoMUCMode").toBool ())
			Ui_.RosterMode_->setCurrentIndex (isMUC ? 1 : 0);

		if (isMUC)
			ProxyModel_->SetMUC (obj);
	}

	void MainWidget::on_RosterMode__currentIndexChanged (int index)
	{
		const bool mucMode = index == 1;

		if (mucMode)
		{
			FstLevelExpands_.clear ();
			SndLevelExpands_.clear ();

			for (int i = 0; i < ProxyModel_->rowCount (); ++i)
			{
				const QModelIndex& accIdx = ProxyModel_->index (i, 0);
				const QString& name = accIdx.data ().toString ();
				FstLevelExpands_ [name] = Ui_.CLTree_->isExpanded (accIdx);

				QMap<QString, bool> groups;
				for (int j = 0, rc = ProxyModel_->rowCount (accIdx);
						j < rc; ++j)
				{
					const QModelIndex& grpIdx = ProxyModel_->index (j, 0, accIdx);
					groups [grpIdx.data ().toString ()] = Ui_.CLTree_->isExpanded (grpIdx);
				}

				SndLevelExpands_ [name] = groups;
			}
		}

		ProxyModel_->SetMUCMode (mucMode);

		if (!mucMode &&
				!FstLevelExpands_.isEmpty () &&
				!SndLevelExpands_.isEmpty ())
		{
			for (int i = 0; i < ProxyModel_->rowCount (); ++i)
			{
				const QModelIndex& accIdx = ProxyModel_->index (i, 0);
				const QString& name = accIdx.data ().toString ();
				if (!FstLevelExpands_.contains (name))
					continue;

				Ui_.CLTree_->setExpanded (accIdx, FstLevelExpands_.take (name));

				const QMap<QString, bool>& groups = SndLevelExpands_.take (name);
				for (int j = 0, rc = ProxyModel_->rowCount (accIdx);
						j < rc; ++j)
				{
					const QModelIndex& grpIdx = ProxyModel_->index (j, 0, accIdx);
					Ui_.CLTree_->setExpanded (grpIdx, groups [grpIdx.data ().toString ()]);
				}
			}
		}
	}

	void MainWidget::menuBarVisibilityToggled ()
	{
		MenuButton_->setVisible (XmlSettingsManager::Instance ().property ("ShowMenuBar").toBool ());
	}

	namespace
	{
		QString BuildPath (const QModelIndex& index)
		{
			if (!index.isValid ())
				return QString ();

			QString path = "CLTreeState/Expanded/" + index.data ().toString ();
			QModelIndex parent = index;
			while ((parent = parent.parent ()).isValid ())
				path.prepend (parent.data ().toString () + "/");

			path = path.toUtf8 ().toBase64 ().replace ('/', '_');

			return path;
		}
	}

	void MainWidget::handleRowsInserted (const QModelIndex& parent, int begin, int end)
	{
		const QAbstractItemModel *clModel = ProxyModel_;
		for (int i = begin; i <= end; ++i)
		{
			const QModelIndex& index = clModel->index (i, 0, parent);
			const Core::CLEntryType type =
					index.data (Core::CLREntryType).value<Core::CLEntryType> ();
			if (type == Core::CLETCategory)
			{
				const QString& path = BuildPath (index);

				const bool expanded = XmlSettingsManager::Instance ().Property (path, true).toBool ();
				if (expanded)
					QMetaObject::invokeMethod (this,
							"expandIndex",
							Qt::QueuedConnection,
							Q_ARG (QPersistentModelIndex, QPersistentModelIndex (index)));

				if (clModel->rowCount (index))
					handleRowsInserted (index, 0, ProxyModel_->rowCount (index) - 1);
			}
			else if (type == Core::CLETAccount)
				QMetaObject::invokeMethod (this,
						"expandIndex",
						Qt::QueuedConnection,
						Q_ARG (QPersistentModelIndex, QPersistentModelIndex (index)));
		}
	}

	void MainWidget::rebuildTreeExpansions ()
	{
		if (Core::Instance ().GetCLModel ()->rowCount ())
			handleRowsInserted (QModelIndex (),
					0, Core::Instance ().GetCLModel ()->rowCount () - 1);
	}

	void MainWidget::expandIndex (const QPersistentModelIndex& pIdx)
	{
		if (!pIdx.isValid ())
			return;

		Ui_.CLTree_->expand (pIdx);
	}

	namespace
	{
		void SetExpanded (const QModelIndex& idx, bool expanded)
		{
			const QString& path = BuildPath (idx);
			if (path.isEmpty ())
				return;

			XmlSettingsManager::Instance ().setProperty (path.toUtf8 (), expanded);
		}
	}

	void MainWidget::on_CLTree__collapsed (const QModelIndex& idx)
	{
		SetExpanded (idx, false);
	}

	void MainWidget::on_CLTree__expanded (const QModelIndex& idx)
	{
		SetExpanded (idx, true);
	}

	void MainWidget::consoleRemoved (QWidget *cwWidget)
	{
		ConsoleWidget *cw = qobject_cast<ConsoleWidget*> (cwWidget);
		Account2CW_.remove (Account2CW_.key (cw));
	}
}
}
