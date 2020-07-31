/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QMainWindow>
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
class QWidgetAction;
class QScreen;

namespace LC
{
	namespace Util
	{
		class XmlSettingsDialog;
	};

	class Core;
	class PluginInfo;
	class PluginManagerDialog;
	class ShortcutManager;
	class MainWindowMenuManager;

	class MainWindow : public QMainWindow
	{
		Q_OBJECT

		Ui::LeechCraft Ui_;

		const bool IsPrimary_;
		const int WindowIdx_;

		QSystemTrayIcon *TrayIcon_ = nullptr;
		bool IsShown_ = true;
		bool WasMaximized_ = false;
		QShortcut *FullScreenShortcut_;
		QShortcut *CloseTabShortcut_;

		QToolBar *QLBar_;

		QToolButton *MenuButton_;
		QWidgetAction *MBAction_;
		MainWindowMenuManager *MenuManager_;

		QToolBar *LeftDockToolbar_;
		QToolBar *RightDockToolbar_;
		QToolBar *TopDockToolbar_;
		QToolBar *BottomDockToolbar_;
	public:
		MainWindow (QScreen *screen, bool isPrimary, int windowIdx);

		void Init ();

		SeparateTabWidget* GetTabWidget () const;
		QSplitter* GetMainSplitter () const;
		void SetAdditionalTitle (const QString&);

		QMenu* GetMainMenu () const;
		void HideMainMenu ();

		QToolBar* GetDockListWidget (Qt::DockWidgetArea) const;

		MainWindowMenuManager* GetMenuManager () const;

		QMenu* createPopupMenu () override;
	public slots:
		void showHideMain ();
		void showMain ();
		void showFirstTime ();

		void handleQuit ();
	protected:
		void closeEvent (QCloseEvent*) override;
		void keyPressEvent (QKeyEvent*) override;

		void dragEnterEvent (QDragEnterEvent*) override;
		void dropEvent (QDropEvent*) override;
	private:
		void InitializeInterface ();
		void SetStatusBar ();
		void ReadSettings ();
		void WriteSettings ();
	private slots:
		void on_ActionAddTask__triggered ();
		void on_ActionNewWindow__triggered ();
		void on_ActionCloseTab__triggered ();
		void handleCloseCurrentTab ();
		void on_ActionSettings__triggered ();
		void on_ActionAboutLeechCraft__triggered ();
		void on_ActionRestart__triggered ();
		void on_ActionQuit__triggered ();
		void on_ActionShowStatusBar__triggered ();
		void on_ActionFullscreenMode__triggered (bool);
		void handleShortcutFullscreenMode ();
		void handleToolButtonStyleChanged ();
		void handleShowTrayIconChanged ();
		void handleNewTabMenuRequested ();
		void handleCurrentTabChanged (int);
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
		void hookGonnaFillMenu (LC::IHookProxy_ptr);
		void hookGonnaFillQuickLaunch (LC::IHookProxy_ptr);
		void hookGonnaShowStatusBar (LC::IHookProxy_ptr, bool);
		void hookTrayIconCreated (LC::IHookProxy_ptr, QSystemTrayIcon*);
		void hookTrayIconVisibilityChanged (LC::IHookProxy_ptr, QSystemTrayIcon*, bool);
	};
}
