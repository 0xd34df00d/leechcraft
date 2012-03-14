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

#ifndef PLUGINS_AZOTH_MAINWIDGET_H
#define PLUGINS_AZOTH_MAINWIDGET_H
#include <QWidget>
#include "interfaces/iaccount.h"
#include "ui_mainwidget.h"

class QToolButton;
class QToolBar;
class QMenu;

namespace LeechCraft
{
namespace Azoth
{
	class SortFilterProxyModel;
	class ConsoleWidget;
	class ServiceDiscoveryWidget;
	class AccountActionsManager;

	class MainWidget : public QWidget
	{
		Q_OBJECT

		Ui::MainWidget Ui_;

		QMenu *MainMenu_;
		QToolButton *MenuButton_;
		SortFilterProxyModel *ProxyModel_;

		QToolButton *FastStatusButton_;
		QAction *ActionCLMode_;
		QAction *ActionShowOffline_;
		QToolBar *BottomBar_;

		QMenu *MenuChangeStatus_;
		QMenu *TrayChangeStatus_;

		AccountActionsManager *AccountActsMgr_;

		QMap<QString, bool> FstLevelExpands_;
		QMap<QString, QMap<QString, bool>> SndLevelExpands_;
	public:
		MainWidget (QWidget* = 0);

		QList<QAction*> GetMenuActions ();
		QMenu* GetChangeStatusMenu () const;
	private:
		void CreateMenu ();
		QMenu* CreateStatusChangeMenu (const char*, bool withCustom = false);
	private slots:
		void updateFastStatusButton (LeechCraft::Azoth::State);
		void treeActivated (const QModelIndex&);
		void on_CLTree__customContextMenuRequested (const QPoint&);
		void handleChangeStatusRequested ();
		void fastStateChangeRequested ();
		void applyFastStatus ();

		void handleEntryActivationType ();
		void handleCatRenameTriggered ();
		void handleSendGroupMsgTriggered ();

		void handleManageBookmarks ();
		void handleAddAccountRequested ();
		void handleAddContactRequested ();

		void handleShowOffline (bool);
		void clearFilter ();

		void handleEntryMadeCurrent (QObject*);
		void resetToWholeMode ();
		void handleCLMode (bool);
		void menuBarVisibilityToggled ();
		void handleStatusIconsChanged ();

		void handleRowsInserted (const QModelIndex&, int, int);
		void rebuildTreeExpansions ();
		void expandIndex (const QPersistentModelIndex&);
		void on_CLTree__expanded (const QModelIndex&);
		void on_CLTree__collapsed (const QModelIndex&);
	signals:
		void gotConsoleWidget (ConsoleWidget*);
		void gotSDWidget (ServiceDiscoveryWidget*);
	};
}
}

#endif
