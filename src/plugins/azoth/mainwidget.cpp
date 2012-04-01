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

#include "mainwidget.h"
#include <QMenu>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QToolButton>
#include <QInputDialog>
#include <QToolBar>
#include <QTimer>
#include <util/util.h>
#include <interfaces/core/icoreproxy.h>
#include "interfaces/iclentry.h"
#include "core.h"
#include "sortfilterproxymodel.h"
#include "setstatusdialog.h"
#include "contactlistdelegate.h"
#include "xmlsettingsmanager.h"
#include "addcontactdialog.h"
#include "chattabsmanager.h"
#include "util.h"
#include "groupsenddialog.h"
#include "actionsmanager.h"
#include "accountactionsmanager.h"
#include "bookmarksmanagerdialog.h"

namespace LeechCraft
{
namespace Azoth
{
	namespace
	{
		class KeyboardRosterFixer : public QObject
		{
			QTreeView *View_;
			bool IsSearching_;
		public:
			KeyboardRosterFixer (QTreeView *view, QObject *parent = 0)
			: QObject (parent)
			, View_ (view)
			, IsSearching_ (false)
			{
			}
		protected:
			bool eventFilter (QObject*, QEvent *e)
			{
				if (e->type () != QEvent::KeyPress &&
					e->type () != QEvent::KeyRelease)
					return false;

				QKeyEvent *ke = static_cast<QKeyEvent*> (e);
				if (!IsSearching_)
				{
					switch (ke->key ())
					{
					case Qt::Key_Space:
					case Qt::Key_Right:
					case Qt::Key_Left:
						qApp->sendEvent (View_, e);
						return true;
					default:
						;
					}
				}

				switch (ke->key ())
				{
				case Qt::Key_Down:
				case Qt::Key_Up:
				case Qt::Key_PageDown:
				case Qt::Key_PageUp:
				case Qt::Key_Enter:
				case Qt::Key_Return:
					IsSearching_ = false;
					qApp->sendEvent (View_, e);
					return true;
				default:
					IsSearching_ = true;
					return false;
				}
			}
		};
	}

	MainWidget::MainWidget (QWidget *parent)
	: QWidget (parent)
	, MainMenu_ (new QMenu (tr ("Azoth menu"), this))
	, MenuButton_ (new QToolButton (this))
	, ProxyModel_ (new SortFilterProxyModel (this))
	, FastStatusButton_ (new QToolButton (this))
	, ActionCLMode_ (new QAction (tr ("CL mode"), this))
	, ActionShowOffline_ (0)
	, BottomBar_ (new QToolBar (tr ("Azoth bar"), this))
	, AccountActsMgr_ (new AccountActionsManager (this, this))
	{
		connect (AccountActsMgr_,
				SIGNAL (gotConsoleWidget (ConsoleWidget*)),
				this,
				SIGNAL (gotConsoleWidget (ConsoleWidget*)));
		connect (AccountActsMgr_,
				SIGNAL (gotSDWidget (ServiceDiscoveryWidget*)),
				this,
				SIGNAL (gotSDWidget (ServiceDiscoveryWidget*)));

		qRegisterMetaType<QPersistentModelIndex> ("QPersistentModelIndex");

		MainMenu_->setIcon (QIcon (":/plugins/azoth/resources/images/azoth.svg"));

		BottomBar_->addWidget (MenuButton_);
		BottomBar_->addWidget (FastStatusButton_);
		FastStatusButton_->setPopupMode (QToolButton::MenuButtonPopup);

		Ui_.setupUi (this);
		Ui_.FilterLine_->setPlaceholderText (tr ("Search..."));
		Ui_.CLTree_->setFocusProxy (Ui_.FilterLine_);

		Ui_.FilterLine_->installEventFilter (new KeyboardRosterFixer (Ui_.CLTree_, this));

		Ui_.CLTree_->setItemDelegate (new ContactListDelegate (Ui_.CLTree_));
		ProxyModel_->setSourceModel (Core::Instance ().GetCLModel ());
		Ui_.CLTree_->setModel (ProxyModel_);

		Ui_.CLTree_->viewport ()->setAcceptDrops (true);

		connect (Core::Instance ().GetChatTabsManager (),
				SIGNAL (entryMadeCurrent (QObject*)),
				this,
				SLOT (handleEntryMadeCurrent (QObject*)),
				Qt::QueuedConnection);

		XmlSettingsManager::Instance ().RegisterObject ("EntryActivationType",
				this, "handleEntryActivationType");
		handleEntryActivationType ();

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
		connect (ProxyModel_,
				SIGNAL (wholeMode ()),
				this,
				SLOT (resetToWholeMode ()));

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

		MenuChangeStatus_->menuAction ()->setProperty ("ActionIcon", "im-status-message-edit");

		FastStatusButton_->setMenu (CreateStatusChangeMenu (SLOT (fastStateChangeRequested ()), true));
		FastStatusButton_->setDefaultAction (new QAction (tr ("Set status"), this));
		updateFastStatusButton (SOffline);
		connect (FastStatusButton_->defaultAction (),
				SIGNAL (triggered ()),
				this,
				SLOT (applyFastStatus ()));

		XmlSettingsManager::Instance ().RegisterObject ("ShowMenuBar",
				this, "menuBarVisibilityToggled");
		menuBarVisibilityToggled ();

		XmlSettingsManager::Instance ().RegisterObject ("StatusIcons",
				this, "handleStatusIconsChanged");
		handleStatusIconsChanged ();

		connect (&Core::Instance (),
				SIGNAL (topStatusChanged (LeechCraft::Azoth::State)),
				this,
				SLOT (updateFastStatusButton (LeechCraft::Azoth::State)));

		qobject_cast<QVBoxLayout*> (layout ())->insertWidget (0, BottomBar_);
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

		QAction *addContact = MainMenu_->addAction (tr ("Add contact..."),
				this,
				SLOT (handleAddContactRequested ()));
		addContact->setProperty ("ActionIcon", "list-add-user");

		QAction *joinConf = MainMenu_->addAction (tr ("Join conference..."),
				&Core::Instance (),
				SLOT (handleMucJoinRequested ()));
		joinConf->setProperty ("ActionIcon", "irc-join-channel");

		MainMenu_->addSeparator ();
		MainMenu_->addAction (tr ("Manage bookmarks..."),
				this,
				SLOT (handleManageBookmarks ()));
		MainMenu_->addSeparator ();
		MainMenu_->addAction (tr ("Add account..."),
				this,
				SLOT (handleAddAccountRequested ()));
		MainMenu_->addSeparator ();

		ActionShowOffline_ = MainMenu_->addAction (tr ("Show offline contacts"));
		ActionShowOffline_->setCheckable (true);
		bool show = XmlSettingsManager::Instance ()
				.Property ("ShowOfflineContacts", true).toBool ();
		ProxyModel_->showOfflineContacts (show);
		ActionShowOffline_->setChecked (show);
		connect (ActionShowOffline_,
				SIGNAL (toggled (bool)),
				this,
				SLOT (handleShowOffline (bool)));

		ActionCLMode_->setCheckable (true);
		ActionCLMode_->setProperty ("ActionIcon", "meeting-attending");
		connect (ActionCLMode_,
				SIGNAL (toggled (bool)),
				this,
				SLOT (handleCLMode (bool)));

		BottomBar_->setToolButtonStyle (Qt::ToolButtonIconOnly);

		auto addBottomAct = [this] (QAction *act)
		{
			const QString& icon = act->property ("ActionIcon").toString ();
			act->setIcon (Core::Instance ().GetProxy ()->GetIcon (icon));
			BottomBar_->addAction (act);
		};
		addBottomAct (addContact);
		addBottomAct (ActionShowOffline_);
		addBottomAct (ActionCLMode_);
	}

	QMenu* MainWidget::CreateStatusChangeMenu (const char *slot, bool withCustom)
	{
		QMenu *result = new QMenu (tr ("Change status"), this);
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
				tr ("Not available"), this, slot)->
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

	void MainWidget::updateFastStatusButton (State state)
	{
		FastStatusButton_->defaultAction ()->setIcon (Core::Instance ().GetIconForState (state));
		FastStatusButton_->setProperty ("Azoth/TargetState",
				QVariant::fromValue<State> (state));
	}

	void MainWidget::treeActivated (const QModelIndex& index)
	{
		if (index.data (Core::CLREntryType).value<Core::CLEntryType> () != Core::CLETContact)
			return;

		Core::Instance ().OpenChat (ProxyModel_->mapToSource (index));
	}

	void MainWidget::on_CLTree__customContextMenuRequested (const QPoint& pos)
	{
		const QModelIndex& index = ProxyModel_->mapToSource (Ui_.CLTree_->indexAt (pos));
		if (!index.isValid ())
			return;

		ActionsManager *manager = Core::Instance ().GetActionsManager ();

		QMenu *menu = new QMenu (tr ("Entry context menu"));
		QList<QAction*> actions;
		switch (index.data (Core::CLREntryType).value<Core::CLEntryType> ())
		{
		case Core::CLETContact:
		{
			QObject *obj = index.data (Core::CLREntryObject).value<QObject*> ();
			ICLEntry *entry = qobject_cast<ICLEntry*> (obj);
			const QList<QAction*>& allActions = manager->GetEntryActions (entry);
			Q_FOREACH (QAction *action, allActions)
				if (manager->GetAreasForAction (action)
						.contains (ActionsManager::CLEAAContactListCtxtMenu) ||
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

			QAction *sendMsg = new QAction (tr ("Send message..."), menu);
			QList<QVariant> entries;
			for (int i = 0, cnt = index.model ()->rowCount (index);
					i < cnt; ++i)
				entries << index.child (i, 0).data (Core::CLREntryObject);
			sendMsg->setProperty ("Azoth/Entries", entries);
			connect (sendMsg,
					SIGNAL (triggered ()),
					this,
					SLOT (handleSendGroupMsgTriggered ()));
			actions << sendMsg;
			break;
		}
		case Core::CLETAccount:
		{
			QVariant objVar = index.data (Core::CLRAccountObject);
			actions << AccountActsMgr_->GetMenuActions (menu, objVar.value<QObject*> ());
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
			actions.prepend (Util::CreateSeparator (menu));
			actions.prepend (MenuChangeStatus_->menuAction ());

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
		menu->deleteLater ();
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
			SetStatusDialog ssd (acc ? acc->GetAccountID () : "global", this);
			if (ssd.exec () != QDialog::Accepted)
				return;

			status = EntryStatus (ssd.GetState (), ssd.GetStatusText ());
		}

		if (acc)
			acc->ChangeState (status);
		else
		{
			Q_FOREACH (IAccount *acc, Core::Instance ().GetAccounts ())
				acc->ChangeState (status);
			updateFastStatusButton (status.State_);
		}
	}

	void MainWidget::fastStateChangeRequested ()
	{
		updateFastStatusButton (sender ()->
					property ("Azoth/TargetState").value<State> ());
		applyFastStatus ();
	}

	void MainWidget::applyFastStatus ()
	{
		State state = FastStatusButton_->
				property ("Azoth/TargetState").value<State> ();

		EntryStatus status (state, QString ());
		Q_FOREACH (IAccount *acc, Core::Instance ().GetAccounts ())
			acc->ChangeState (status);
	}

	void MainWidget::handleEntryActivationType ()
	{
		disconnect (Ui_.CLTree_,
				0,
				this,
				SLOT (treeActivated (const QModelIndex&)));
		disconnect (Ui_.CLTree_,
				0,
				this,
				SLOT (clearFilter ()));

		const char *signal = 0;
		const QString& actType = XmlSettingsManager::Instance ()
				.property ("EntryActivationType").toString ();

		if (actType == "click")
			signal = SIGNAL (clicked (const QModelIndex&));
		else if (actType == "dclick")
			signal = SIGNAL (doubleClicked (const QModelIndex&));
		else
			signal = SIGNAL (activated (const QModelIndex&));

		connect (Ui_.CLTree_,
				signal,
				this,
				SLOT (treeActivated (const QModelIndex&)));
		connect (Ui_.CLTree_,
				signal,
				this,
				SLOT (clearFilter ()));
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

	void MainWidget::handleSendGroupMsgTriggered ()
	{
		QList<QObject*> entries;

		Q_FOREACH (const QVariant& entryVar,
				sender ()->property ("Azoth/Entries").toList ())
			entries << entryVar.value<QObject*> ();

		GroupSendDialog *dlg = new GroupSendDialog (entries, this);
		dlg->setAttribute (Qt::WA_DeleteOnClose, true);
		dlg->show ();
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
		std::unique_ptr<AddContactDialog> dia (new AddContactDialog (0, this));
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
			ActionCLMode_->setChecked (isMUC);

		if (isMUC)
			ProxyModel_->SetMUC (obj);
	}

	void MainWidget::resetToWholeMode ()
	{
		ActionCLMode_->setChecked (false);
	}

	void MainWidget::handleCLMode (bool mucMode)
	{
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
		BottomBar_->setVisible (XmlSettingsManager::Instance ().property ("ShowMenuBar").toBool ());
	}

	void MainWidget::handleStatusIconsChanged ()
	{
		ActionShowOffline_->setIcon (Core::Instance ().GetIconForState (SOffline));
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

				const bool expanded = ProxyModel_->IsMUCMode () ||
						XmlSettingsManager::Instance ().Property (path, true).toBool ();
				if (expanded)
					QMetaObject::invokeMethod (this,
							"expandIndex",
							Qt::QueuedConnection,
							Q_ARG (QPersistentModelIndex, QPersistentModelIndex (index)));

				if (clModel->rowCount (index))
					handleRowsInserted (index, 0, ProxyModel_->rowCount (index) - 1);
			}
			else if (type == Core::CLETAccount)
			{
				QMetaObject::invokeMethod (this,
						"expandIndex",
						Qt::QueuedConnection,
						Q_ARG (QPersistentModelIndex, QPersistentModelIndex (index)));

				if (clModel->rowCount (index))
					handleRowsInserted (index, 0, ProxyModel_->rowCount (index) - 1);
			}
		}
	}

	void MainWidget::rebuildTreeExpansions ()
	{
		if (ProxyModel_->rowCount ())
			handleRowsInserted (QModelIndex (), 0, ProxyModel_->rowCount () - 1);
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
}
}
