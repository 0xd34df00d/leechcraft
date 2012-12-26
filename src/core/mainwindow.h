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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QDialog>
#include <QSettings>
#include <QSystemTrayIcon>
#include <QList>
#include <QModelIndex>
#include <QToolButton>
#include "ui_leechcraft.h"
#include "interfaces/core/ihookproxy.h"

class QLabel;
class QDockWidget;
class QModelIndex;
class QToolBar;
class IShortcutProxy;
class QToolButton;
class QShortcut;
class QSplashScreen;
class QSystemTrayIcon;

namespace LeechCraft
{
	namespace Util
	{
		class XmlSettingsDialog;
	};

	class Core;
	class PluginInfo;
	class PluginManagerDialog;
	class ShortcutManager;
	class ToolbarGuard;

	class MainWindow : public QMainWindow
	{
		Q_OBJECT

		Ui::LeechCraft Ui_;

		QSystemTrayIcon *TrayIcon_;
		bool IsShown_;
		bool WasMaximized_;
		ToolbarGuard *Guard_;
		QShortcut *FullScreenShortcut_;
		QShortcut *CloseTabShortcut_;
		bool IsQuitting_;

		QToolBar *QLBar_;

		QMenu *MenuView_;
		QMenu *MenuTools_;

		bool IsToolBarVisible_;
	public:
		MainWindow (QWidget *parent = 0, Qt::WFlags flags = 0);
		void Init ();
		virtual ~MainWindow ();

		SeparateTabWidget* GetTabWidget () const;
		QSplitter* GetMainSplitter () const;
		void SetAdditionalTitle (const QString&);
		ToolbarGuard* GetGuard () const;

		QMenu* GetMainMenu () const;
		void HideMainMenu ();

		QWidget* GetDockListWidget (Qt::DockWidgetArea) const;

		void AddMenus (const QMap<QString, QList<QAction*>>&);
		void RemoveMenus (const QMap<QString, QList<QAction*>>&);
	public slots:
		void catchError (QString);
		void showHideMain ();

		void handleQuit ();
	protected:
		virtual void closeEvent (QCloseEvent*);
		virtual void keyPressEvent (QKeyEvent*);
		virtual void keyReleaseEvent (QKeyEvent*);

		virtual void dragEnterEvent (QDragEnterEvent*);
		virtual void dropEvent (QDropEvent*);
	private:
		void InitializeInterface ();
		void SetStatusBar ();
		void ReadSettings ();
		void WriteSettings ();
	private slots:
		void on_ActionAddTask__triggered ();
		void on_ActionCloseTab__triggered ();
		void handleCloseCurrentTab ();
		void on_ActionSettings__triggered ();
		void on_ActionAboutLeechCraft__triggered ();
		void on_ActionRestart__triggered ();
		void on_ActionQuit__triggered ();
		void on_ActionShowStatusBar__triggered ();
		void on_ActionMenu__triggered ();
		void on_ActionFullscreenMode__triggered (bool);
		void on_MainTabWidget__currentChanged (int);
		void on_ActionShowToolBar__triggered (bool);
		void handleShortcutFullscreenMode ();
		void handleToolButtonStyleChanged ();
		void handleToolBarManipulationChanged ();
		void handleShowTrayIconChanged ();
		void handleNewTabMenuRequested ();
		void handleRestoreActionAdded (QAction*);
		void handleTrayIconActivated (QSystemTrayIcon::ActivationReason);
		void doDelayedInit ();
	private:
		void FillQuickLaunch ();
		void FillTray ();
		void FillToolMenu ();
		void InitializeShortcuts ();
		void ShowMenuAndBar (bool);
	signals:
		void hookGonnaFillMenu (LeechCraft::IHookProxy_ptr);
		void hookGonnaFillQuickLaunch (LeechCraft::IHookProxy_ptr);
		void hookTrayIconCreated (LeechCraft::IHookProxy_ptr, QSystemTrayIcon*);
		void hookTrayIconVisibilityChanged (LeechCraft::IHookProxy_ptr, QSystemTrayIcon*, bool);
	};
};

#endif

