/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "mainwidget.h"
#include <QMenu>
#include <QMainWindow>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QToolButton>
#include <QToolBar>
#include <QShortcut>
#include <QToolTip>
#include <QTimer>
#include <util/util.h>
#include <util/gui/clearlineeditaddon.h>
#include <util/shortcuts/shortcutmanager.h>
#include <util/sll/slotclosure.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include "interfaces/azoth/iclentry.h"
#include "components/dialogs/addcontactdialog.h"
#include "components/dialogs/bookmarksmanagerdialog.h"
#include "components/dialogs/setstatusdialog.h"
#include "core.h"
#include "sortfilterproxymodel.h"
#include "contactlistdelegate.h"
#include "xmlsettingsmanager.h"
#include "chattabsmanager.h"
#include "util.h"
#include "actionsmanager.h"
#include "accountactions.h"
#include "categoryactions.h"
#include "keyboardrosterfixer.h"
#include "userslistwidget.h"
#include "resourcesmanager.h"

namespace LC
{
namespace Azoth
{
	MainWidget::MainWidget (QWidget *parent)
	: QWidget (parent)
	, MainMenu_ (new QMenu (tr ("Azoth menu"), this))
	, MenuButton_ (new QToolButton (this))
	, ProxyModel_ (new SortFilterProxyModel (this))
	, ActionCLMode_ (new QAction (tr ("CL mode"), this))
	, ActionShowOffline_ (0)
	, BottomBar_ (new QToolBar (tr ("Azoth bar"), this))
	{
		qRegisterMetaType<QPersistentModelIndex> ("QPersistentModelIndex");

		MainMenu_->setIcon (QIcon ("lcicons:/plugins/azoth/resources/images/azoth.svg"));

		BottomBar_->addWidget (MenuButton_);

		Ui_.setupUi (this);
		new Util::ClearLineEditAddon (Core::Instance ().GetProxy (), Ui_.FilterLine_);
		Ui_.FilterLine_->setPlaceholderText (tr ("Search..."));
		Ui_.CLTree_->setFocusProxy (Ui_.FilterLine_);

		new KeyboardRosterFixer (Ui_.FilterLine_, Ui_.CLTree_, this);

		Ui_.CLTree_->setItemDelegate (new ContactListDelegate (Ui_.CLTree_));
		ProxyModel_->setSourceModel (Core::Instance ().GetCLModel ());
		Ui_.CLTree_->setModel (ProxyModel_);

		Ui_.CLTree_->viewport ()->setAcceptDrops (true);

		connect (Core::Instance ().GetChatTabsManager (),
				SIGNAL (entryMadeCurrent (QObject*)),
				this,
				SLOT (handleEntryMadeCurrent (QObject*)));
		connect (Core::Instance ().GetChatTabsManager (),
				SIGNAL (entryLostCurrent (QObject*)),
				this,
				SLOT (handleEntryLostCurrent (QObject*)));

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

		if (ProxyModel_->rowCount ())
			QMetaObject::invokeMethod (this,
					"handleRowsInserted",
					Qt::QueuedConnection,
					Q_ARG (QModelIndex, QModelIndex ()),
					Q_ARG (int, 0),
					Q_ARG (int, ProxyModel_->rowCount () - 1));

		CreateMenu ();
		MenuButton_->setMenu (MainMenu_);
		MenuButton_->setIcon (MainMenu_->icon ());
		MenuButton_->setPopupMode (QToolButton::InstantPopup);

		ActionDeleteSelected_ = new QAction (this);
		ActionDeleteSelected_->setShortcut (Qt::Key_Delete);
		ActionDeleteSelected_->setShortcutContext (Qt::WidgetWithChildrenShortcut);
		connect (ActionDeleteSelected_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleDeleteSelected ()));
		addAction (ActionDeleteSelected_);

		XmlSettingsManager::Instance ().RegisterObject ("ShowMenuBar",
				this, "menuBarVisibilityToggled");
		menuBarVisibilityToggled ();

		XmlSettingsManager::Instance ().RegisterObject ("StatusIcons",
				this, "handleStatusIconsChanged");
		handleStatusIconsChanged ();

		qobject_cast<QVBoxLayout*> (layout ())->insertWidget (0, BottomBar_);

		auto sm = Core::Instance ().GetShortcutManager ();
		auto listShortcut = new QShortcut (QString ("Alt+C"),
				this, SLOT (showAllUsersList ()));
		listShortcut->setContext (Qt::ApplicationShortcut);
		sm->RegisterShortcut ("org.LeechCraft.Azoth.AllUsersList",
				{
					tr ("Show all users list"),
					{ "Alt+C" },
					Core::Instance ().GetProxy ()->GetIconThemeManager ()->GetIcon ("system-users")
				},
				listShortcut);

		new Util::SlotClosure<Util::NoDeletePolicy>
		{
			[this]
			{
				const auto& pos = QCursor::pos ();
				const auto widget = Ui_.CLTree_->viewport ();
				QHelpEvent event { QEvent::ToolTip, widget->mapFromGlobal (pos), pos };
				QCoreApplication::sendEvent (widget, &event);
			},
			Core::Instance ().GetCLModel (),
			SIGNAL (rebuiltTooltip ()),
			this
		};
	}

	QList<QAction*> MainWidget::GetMenuActions ()
	{
		return MainMenu_->actions ();
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
		ActionCLMode_->setShortcut (QString ("Ctrl+Shift+R"));
		Core::Instance ().GetShortcutManager ()->RegisterAction ("org.LeechCraft.Azoth.CLMode", ActionCLMode_);
		connect (ActionCLMode_,
				SIGNAL (toggled (bool)),
				this,
				SLOT (handleCLMode (bool)));

		BottomBar_->setToolButtonStyle (Qt::ToolButtonIconOnly);

		auto addBottomAct = [this] (QAction *act)
		{
			const QString& icon = act->property ("ActionIcon").toString ();
			act->setIcon (Core::Instance ().GetProxy ()->GetIconThemeManager ()->GetIcon (icon));
			BottomBar_->addAction (act);
		};
		addBottomAct (addContact);
		addBottomAct (ActionShowOffline_);
		addBottomAct (ActionCLMode_);
	}

	void MainWidget::handleAccountVisibilityChanged ()
	{
		ProxyModel_->invalidate ();
	}

	void MainWidget::treeActivated (const QModelIndex& index)
	{
		if (index.data (Core::CLREntryType).value<Core::CLEntryType> () != Core::CLETContact)
			return;

		if (QApplication::keyboardModifiers () & Qt::CTRL)
			if (auto tab = Core::Instance ().GetChatTabsManager ()->GetActiveChatTab ())
			{
				auto text = index.data ().toString ();

				auto edit = tab->getMsgEdit ();
				if (edit->toPlainText ().isEmpty ())
					text += XmlSettingsManager::Instance ()
							.property ("PostAddressText").toString ();
				else
					text.prepend (" ");

				tab->appendMessageText (text);
				return;
			}

		Core::Instance ().OpenChat (ProxyModel_->mapToSource (index));
	}

	void MainWidget::showAllUsersList ()
	{
		QList<QObject*> entries;
		int accCount = 0;
		for (auto acc : Core::Instance ().GetAccounts ())
		{
			if (!acc->IsShownInRoster ())
				continue;

			++accCount;
			const auto& accEntries = acc->GetCLEntries ();
			std::copy_if (accEntries.begin (), accEntries.end (), std::back_inserter (entries),
					[] (QObject *entryObj) -> bool
					{
						auto entry = qobject_cast<ICLEntry*> (entryObj);
						return entry->GetEntryType () != ICLEntry::EntryType::PrivateChat;
					});
		}

		UsersListWidget w (entries,
				[accCount] (ICLEntry *entry) -> QString
				{
					QString text = entry->GetEntryName ();
					if (text != entry->GetHumanReadableID ())
						text += " (" + entry->GetHumanReadableID () + ")";

					if (accCount <= 1)
						return text;

					auto acc = entry->GetParentAccount ();
					return text + " [" + acc->GetAccountName () + "]";
				},
				this);
		if (w.exec () != QDialog::Accepted)
			return;

		if (auto entry = w.GetActivatedParticipant ())
			Core::Instance ().GetChatTabsManager ()->
					OpenChat (qobject_cast<ICLEntry*> (entry), true);
	}

	namespace
	{
		QList<ICLEntry*> GetCategoryEntries (const QModelIndex& index)
		{
			auto model = index.model ();
			const auto numEntries = model->rowCount (index);

			QList<ICLEntry*> entries;
			entries.reserve (numEntries);
			for (int i = 0; i < numEntries; ++i)
			{
				const auto entryObj = model->index (i, 0, index).data (Core::CLREntryObject).value<QObject*> ();
				entries << qobject_cast<ICLEntry*> (entryObj);
			}
			return entries;
		}

		QList<ICLEntry*> GetCategoryEntries (const QModelIndex& index, QModelIndexList selection)
		{
			auto result = GetCategoryEntries (index);
			for (const auto& selIdx : selection)
				if (selIdx != index && selIdx.data (Core::CLREntryType).toInt () == Core::CLETCategory)
					result += GetCategoryEntries (selIdx);
			return result;
		}
	}

	void MainWidget::on_CLTree__customContextMenuRequested (const QPoint& pos)
	{
		const QModelIndex& index = Ui_.CLTree_->indexAt (pos);
		if (!index.isValid ())
			return;

		QMenu menu { tr ("Entry context menu") };
		switch (index.data (Core::CLREntryType).value<Core::CLEntryType> ())
		{
		case Core::CLETContact:
		{
			QList<QAction*> actions;

			const auto manager = Core::Instance ().GetActionsManager ();
			auto rows = Ui_.CLTree_->selectionModel ()->selectedRows ();
			if (!rows.contains (index))
				rows << index;

			if (rows.size () == 1)
			{
				QObject *obj = index.data (Core::CLREntryObject).value<QObject*> ();
				ICLEntry *entry = qobject_cast<ICLEntry*> (obj);

				const auto& allActions = manager->GetEntryActions (entry);
				for (auto action : allActions)
				{
					const auto& areas = manager->GetAreasForAction (action);
					if (action->isSeparator () ||
							areas.contains (ActionsManager::CLEAAContactListCtxtMenu))
						actions << action;
				}
			}
			else
			{
				QList<ICLEntry*> entries;
				for (const auto& row : rows)
				{
					const auto entryObj = row.data (Core::CLREntryObject).value<QObject*> ();
					const auto entry = qobject_cast<ICLEntry*> (entryObj);
					if (entry)
						entries << entry;
				}

				const auto& allActions = manager->CreateEntriesActions (entries, &menu);
				for (auto action : allActions)
				{
					const auto& areas = manager->GetAreasForAction (action);
					if (action->isSeparator () ||
							areas.contains (ActionsManager::CLEAAContactListCtxtMenu))
						actions << action;
				}
			}
			menu.addActions (actions);
			break;
		}
		case Core::CLETCategory:
			Actions::PopulateMenu (&menu,
					Actions::CategoryInfo
					{
						.Name_ = index.data ().toString (),
						.Account_ = qobject_cast<IAccount*> (index.parent ().data (Core::CLRAccountObject).value<QObject*> ()),
						.Entries_ = GetCategoryEntries (index, Ui_.CLTree_->selectionModel ()->selectedRows ()),
						.UnreadCount_ = index.data (Core::CLRUnreadMsgCount).toInt (),
					});
			break;
		case Core::CLETAccount:
			Actions::PopulateMenu (&menu, index.data (Core::CLRAccountObject).value<IAccount*> ());
			break;
		default:
			return;
		}

		menu.exec (Ui_.CLTree_->mapToGlobal (pos));
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

	void MainWidget::handleManageBookmarks ()
	{
		BookmarksManagerDialog *dia = new BookmarksManagerDialog (this);
		dia->show ();
	}

	void MainWidget::handleAddAccountRequested ()
	{
		InitiateAccountAddition (this);
	}

	void MainWidget::handleAddContactRequested ()
	{
		AddContactDialog dia { 0, this };
		if (dia.exec () != QDialog::Accepted)
			return;

		if (!dia.GetSelectedAccount ())
			return;

		dia.GetSelectedAccount ()->RequestAuth (dia.GetContactID (),
					dia.GetReason (),
					dia.GetNick (),
					dia.GetGroups ());
	}

	void MainWidget::handleShowOffline (bool show)
	{
		XmlSettingsManager::Instance ().setProperty ("ShowOfflineContacts", show);
		ProxyModel_->showOfflineContacts (show);
	}

	void MainWidget::clearFilter ()
	{
		if (!XmlSettingsManager::Instance ().property ("ClearSearchAfterFocus").toBool ())
			return;

		if (!Ui_.FilterLine_->text ().isEmpty ())
			Ui_.FilterLine_->setText (QString ());
	}

	void MainWidget::handleDeleteSelected ()
	{
		const auto& idx = ProxyModel_->mapToSource (Ui_.CLTree_->currentIndex ());
		if (!idx.isValid ())
			return;

		if (idx.data (Core::CLREntryType).value<Core::CLEntryType> () != Core::CLETContact)
			return;

		const auto& obj = idx.data (Core::CLREntryObject).value<QObject*> ();
		auto entry = qobject_cast<ICLEntry*> (obj);
		auto acc = entry ? entry->GetParentAccount () : nullptr;
		if (!entry || !acc)
		{
			qWarning () << Q_FUNC_INFO
					<< "no entry or account";
			return;
		}

		if (QMessageBox::question (this,
				"LeechCraft",
				tr ("Are you sure you want to remove %1 from roster?")
					.arg (entry->GetEntryName ()),
				QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
			return;

		acc->RemoveEntry (obj);
	}

	void MainWidget::handleEntryMadeCurrent (QObject *obj)
	{
		auto entry = qobject_cast<ICLEntry*> (obj);
		if (entry && entry->GetEntryType () == ICLEntry::EntryType::PrivateChat)
			obj = entry->GetParentCLEntryObject ();

		const bool isMUC = qobject_cast<IMUCEntry*> (obj);

		if (XmlSettingsManager::Instance ().property ("AutoMUCMode").toBool ())
			ActionCLMode_->setChecked (isMUC);

		if (isMUC)
			ProxyModel_->SetMUC (obj);
	}

	void MainWidget::handleEntryLostCurrent (QObject*)
	{
		if (XmlSettingsManager::Instance ().property ("AutoMUCMode").toBool ())
			ActionCLMode_->setChecked (false);
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
		ActionShowOffline_->setIcon (ResourcesManager::Instance ().GetIconForState (SOffline));
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
		for (int i = begin; i <= end; ++i)
		{
			const auto& index = ProxyModel_->index (i, 0, parent);
			if (!index.isValid ())
			{
				qWarning () << Q_FUNC_INFO
						<< "invalid index"
						<< parent
						<< i
						<< "in"
						<< begin
						<< end;
				continue;
			}

			const auto type = index.data (Core::CLREntryType).value<Core::CLEntryType> ();
			if (type == Core::CLETCategory)
			{
				const QString& path = BuildPath (index);

				const bool expanded = ProxyModel_->IsMUCMode () ||
						XmlSettingsManager::Instance ().Property (path.toStdString (), true).toBool ();
				if (expanded)
					QMetaObject::invokeMethod (this,
							"expandIndex",
							Qt::QueuedConnection,
							Q_ARG (QPersistentModelIndex, QPersistentModelIndex (index)));

				if (ProxyModel_->rowCount (index))
					handleRowsInserted (index, 0, ProxyModel_->rowCount (index) - 1);
			}
			else if (type == Core::CLETAccount)
			{
				QMetaObject::invokeMethod (this,
						"expandIndex",
						Qt::QueuedConnection,
						Q_ARG (QPersistentModelIndex, QPersistentModelIndex (index)));

				if (ProxyModel_->rowCount (index))
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
		if (pIdx.isValid ())
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
