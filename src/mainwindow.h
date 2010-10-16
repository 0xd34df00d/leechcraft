/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

class QLabel;
class QDockWidget;
class QModelIndex;
class QToolBar;
class IShortcutProxy;
class QToolButton;
class QShortcut;

namespace LeechCraft
{
	namespace Util
	{
		class GraphWidget;
		class XmlSettingsDialog;
	};

	class Core;
	class PluginInfo;
	class PluginManagerDialog;
	class FancyPopupManager;
	class SettingsSink;
	class ShortcutManager;
	class LogToolBox;
	class ToolbarGuard;
	class GlanceShower;

	class NewTabButton : public QToolButton
	{
	public:
		explicit NewTabButton (QWidget* parent=0) : QToolButton (parent) {};
	protected:
		void mousePressEvent (QMouseEvent *event);
	};

	class MainWindow : public QMainWindow
	{
		Q_OBJECT

		Ui::LeechCraft Ui_;

		QSystemTrayIcon *TrayIcon_;
		QLabel *DownloadSpeed_;
		QLabel *UploadSpeed_;
		QLabel *Clock_;
		Util::GraphWidget *SpeedGraph_;
		LeechCraft::Util::XmlSettingsDialog *XmlSettingsDialog_;
		SettingsSink *SettingsSink_;
		ShortcutManager *ShortcutManager_;
		FancyPopupManager *FancyPopupManager_;
		LeechCraft::LogToolBox *LogToolBox_;
		bool IsShown_;
		bool WasMaximized_;
		QString LanguageOnLoad_;
		ToolbarGuard *Guard_;
		QToolBar *PluginsActionsBar_;
		GlanceShower *Glance_;
		QToolButton *NewTabButton_;
		QShortcut *FullScreenShortcut_;
	public:
		MainWindow (QWidget *parent = 0, Qt::WFlags flags = 0);
		virtual ~MainWindow ();
		TabWidget* GetTabWidget () const;
		const IShortcutProxy* GetShortcutProxy () const;
		void SetAdditionalTitle (const QString&);
		ToolbarGuard* GetGuard () const;
		FancyPopupManager* GetFancyPopupManager () const;

		void AddMenus (const QMap<QString, QList<QAction*> >&);
		void RemoveMenus (const QMap<QString, QList<QAction*> >&);
	public slots:
		void catchError (QString);
	protected:
		virtual void closeEvent (QCloseEvent*);
	private:
		void InitializeInterface ();
		void SetStatusBar ();
		void ReadSettings ();
		void WriteSettings ();
	private slots:
		void on_ActionAddTask__triggered ();
		void on_ActionNewTab__triggered ();
		void on_ActionCloseTab__triggered ();
		void on_ActionGlance__triggered ();
		void on_ActionSettings__triggered ();
		void on_ActionAboutLeechCraft__triggered ();
		void on_ActionQuit__triggered ();
		void on_ActionShowStatusBar__triggered ();
		void on_ActionMenu__triggered ();
		void handleQuit ();
		void handleLanguage ();
		void on_ActionFullscreenMode__triggered (bool);
		void on_ActionLogger__triggered ();
		void on_MainTabWidget__currentChanged (int);
		void on_ShortcutFullscreenMode__triggered();
		void handleToolButtonStyleChanged ();
		void handleIconSize ();
		void handleShowMenuBarAsButton ();
		void handleNewTabMenuRequested ();
		void updateSpeedIndicators ();
		void updateClock ();
		void showHideMain ();
		void handleTrayIconActivated (QSystemTrayIcon::ActivationReason);
		void updateIconSet ();
		void doDelayedInit ();
	private:
		void FillTray ();
		void FillToolMenu ();
		void InitializeShortcuts ();
		void InitializeDataSources ();
		void ShowMenuAndBar (bool);
	};
};

#endif

