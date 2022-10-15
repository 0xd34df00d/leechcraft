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
	class ExpansionStateManager;

	class MainWidget : public QWidget
	{
		Q_OBJECT

		Ui::MainWidget Ui_;

		QMenu *MainMenu_ = nullptr;
		QToolButton *MenuButton_ = nullptr;
		SortFilterProxyModel *ProxyModel_ = nullptr;
		ExpansionStateManager *ExpansionStateMgr_ = nullptr;

		QAction *ActionCLMode_;
		QAction *ActionShowOffline_;
		QAction *ActionDeleteSelected_;
		QToolBar *ButtonsBar_;
	public:
		MainWidget (QWidget* = 0);

		QList<QAction*> GetMenuActions ();
	private:
		void CreateMenu ();

		void TreeActivated (const QModelIndex&);
	public slots:
		void handleAccountVisibilityChanged ();
	private slots:
		void showAllUsersList ();
		void on_CLTree__customContextMenuRequested (const QPoint&);

		void handleManageBookmarks ();
		void handleAddAccountRequested ();
		void handleAddContactRequested ();

		void handleShowOffline (bool);

		void handleDeleteSelected ();

		void handleEntryMadeCurrent (QObject*);
		void handleEntryLostCurrent (QObject*);
		void menuBarVisibilityToggled ();
		void handleStatusIconsChanged ();
	};
}
}
