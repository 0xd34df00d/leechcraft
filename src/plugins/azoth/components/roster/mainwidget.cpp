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
#include "components/dialogs/userslistwidget.h"
#include "keyboardrosterfixer.h"
#include "expansionstatemanager.h"
#include "core.h"
#include "sortfilterproxymodel.h"
#include "contactlistdelegate.h"
#include "xmlsettingsmanager.h"
#include "chattabsmanager.h"
#include "util.h"
#include "actionsmanager.h"
#include "accountactions.h"
#include "categoryactions.h"
#include "resourcesmanager.h"
#include "roles.h"

namespace LC
{
namespace Azoth
{
	namespace
	{
		auto EntryActivationType2Signal (const QString& type)
		{
			if (type == "click")
				return &QTreeView::clicked;
			if (type == "dclick")
				return &QTreeView::doubleClicked;
			return &QTreeView::activated;
		}
	}

	MainWidget::MainWidget (QWidget *parent)
	: QWidget (parent)
	, MainMenu_ (new QMenu (tr ("Azoth menu"), this))
	, MenuButton_ (new QToolButton (this))
	, ProxyModel_ (new SortFilterProxyModel (this))
	, ActionCLMode_ (new QAction (tr ("CL mode"), this))
	, ActionShowOffline_ (0)
	, ButtonsBar_ (new QToolBar (tr ("Azoth bar"), this))
	{
		MainMenu_->setIcon (QIcon ("lcicons:/plugins/azoth/resources/images/azoth.svg"));

		ButtonsBar_->addWidget (MenuButton_);

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
				this,
				[this, conn = std::make_shared<QMetaObject::Connection> ()] (const QVariant& var)
				{
					disconnect (*conn);
					*conn = connect (Ui_.CLTree_,
							EntryActivationType2Signal (var.toString ()),
							this,
							&MainWidget::TreeActivated);
				});

		connect (Ui_.FilterLine_,
				SIGNAL (textChanged (const QString&)),
				ProxyModel_,
				SLOT (setFilterFixedString (const QString&)));

		ExpansionStateMgr_ = new ExpansionStateManager { *ProxyModel_, *Ui_.CLTree_, this };

		connect (ProxyModel_,
				&SortFilterProxyModel::wholeMode,
				this,
				[this] { ActionCLMode_->setChecked (false); });

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

		qobject_cast<QVBoxLayout*> (layout ())->insertWidget (0, ButtonsBar_);

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
				&QAction::toggled,
				ExpansionStateMgr_,
				&ExpansionStateManager::SetMucMode);

		ButtonsBar_->setToolButtonStyle (Qt::ToolButtonIconOnly);

		auto addBottomAct = [this] (QAction *act)
		{
			const QString& icon = act->property ("ActionIcon").toString ();
			act->setIcon (Core::Instance ().GetProxy ()->GetIconThemeManager ()->GetIcon (icon));
			ButtonsBar_->addAction (act);
		};
		addBottomAct (addContact);
		addBottomAct (ActionShowOffline_);
		addBottomAct (ActionCLMode_);
	}

	void MainWidget::handleAccountVisibilityChanged ()
	{
		ProxyModel_->invalidate ();
	}

	void MainWidget::TreeActivated (const QModelIndex& index)
	{
		if (index.data (CLREntryType).value<CLEntryType> () != CLETContact)
			return;

		if (XmlSettingsManager::Instance ().property ("ClearSearchAfterFocus").toBool () &&
				!Ui_.FilterLine_->text ().isEmpty ())
			Ui_.FilterLine_->setText (QString ());

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
				const auto entryObj = model->index (i, 0, index).data (CLREntryObject).value<QObject*> ();
				entries << qobject_cast<ICLEntry*> (entryObj);
			}
			return entries;
		}

		QList<ICLEntry*> GetCategoryEntries (const QModelIndex& index, QModelIndexList selection)
		{
			auto result = GetCategoryEntries (index);
			for (const auto& selIdx : selection)
				if (selIdx != index && selIdx.data (CLREntryType).toInt () == CLETCategory)
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
		switch (index.data (CLREntryType).value<CLEntryType> ())
		{
		case CLETContact:
		{
			QList<QAction*> actions;

			const auto manager = Core::Instance ().GetActionsManager ();
			auto rows = Ui_.CLTree_->selectionModel ()->selectedRows ();
			if (!rows.contains (index))
				rows << index;

			if (rows.size () == 1)
			{
				QObject *obj = index.data (CLREntryObject).value<QObject*> ();
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
					const auto entryObj = row.data (CLREntryObject).value<QObject*> ();
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
		case CLETCategory:
			Actions::PopulateMenu (&menu,
					Actions::CategoryInfo
					{
						.Name_ = index.data ().toString (),
						.Account_ = qobject_cast<IAccount*> (index.parent ().data (CLRAccountObject).value<QObject*> ()),
						.Entries_ = GetCategoryEntries (index, Ui_.CLTree_->selectionModel ()->selectedRows ()),
						.UnreadCount_ = index.data (CLRUnreadMsgCount).toInt (),
					});
			break;
		case CLETAccount:
			Actions::PopulateMenu (&menu, index.data (CLRAccountObject).value<IAccount*> ());
			break;
		default:
			return;
		}

		menu.exec (Ui_.CLTree_->mapToGlobal (pos));
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

	void MainWidget::handleDeleteSelected ()
	{
		const auto& idx = ProxyModel_->mapToSource (Ui_.CLTree_->currentIndex ());
		if (!idx.isValid ())
			return;

		if (idx.data (CLREntryType).value<CLEntryType> () != CLETContact)
			return;

		const auto& obj = idx.data (CLREntryObject).value<QObject*> ();
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

	void MainWidget::menuBarVisibilityToggled ()
	{
		BottomBar_->setVisible (XmlSettingsManager::Instance ().property ("ShowMenuBar").toBool ());
	}

	void MainWidget::handleStatusIconsChanged ()
	{
		ActionShowOffline_->setIcon (ResourcesManager::Instance ().GetIconForState (SOffline));
	}
}
}
