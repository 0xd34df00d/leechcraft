/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include "interfaces/azoth/iaccount.h"
#include "ui_mainwidget.h"

class QToolButton;
class QToolBar;
class QMenu;

namespace LC
{
namespace Azoth
{
	class SortFilterProxyModel;
	class ConsoleWidget;
	class ServiceDiscoveryWidget;
	class MicroblogsTab;
	class AccountActionsManager;

	class MainWidget : public QWidget
	{
		Q_OBJECT

		Ui::MainWidget Ui_;

		AccountActionsManager * const AccountActsMgr_;

		QMenu *MainMenu_;
		QToolButton *MenuButton_;
		SortFilterProxyModel *ProxyModel_;

		QAction *ActionCLMode_;
		QAction *ActionShowOffline_;
		QAction *ActionDeleteSelected_;
		QToolBar *BottomBar_;

		QMap<QString, bool> FstLevelExpands_;
		QMap<QString, QMap<QString, bool>> SndLevelExpands_;
	public:
		MainWidget (AccountActionsManager*, QWidget* = 0);

		QList<QAction*> GetMenuActions ();
	private:
		void CreateMenu ();
	public slots:
		void handleAccountVisibilityChanged ();
	private slots:
		void treeActivated (const QModelIndex&);
		void showAllUsersList ();
		void on_CLTree__customContextMenuRequested (const QPoint&);

		void handleEntryActivationType ();

		void handleCatRenameTriggered ();
		void handleSendGroupMsgTriggered ();
		void handleMarkAllTriggered ();
		void handleRemoveChildrenTriggered ();

		void handleManageBookmarks ();
		void handleAddAccountRequested ();
		void handleAddContactRequested ();

		void handleShowOffline (bool);
		void clearFilter ();

		void handleDeleteSelected ();

		void handleEntryMadeCurrent (QObject*);
		void handleEntryLostCurrent (QObject*);
		void resetToWholeMode ();
		void handleCLMode (bool);
		void menuBarVisibilityToggled ();
		void handleStatusIconsChanged ();

		void handleRowsInserted (const QModelIndex&, int, int);
		void rebuildTreeExpansions ();
		void expandIndex (const QPersistentModelIndex&);
		void on_CLTree__expanded (const QModelIndex&);
		void on_CLTree__collapsed (const QModelIndex&);
	};
}
}
