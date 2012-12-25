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

#include "mainwindow.h"
#include <iostream>
#include <algorithm>
#include <QMessageBox>
#include <QCloseEvent>
#include <QCursor>
#include <QShortcut>
#include <QMenu>
#include <QSplashScreen>
#include <QTime>
#include <QDockWidget>
#include <util/util.h>
#include <util/defaulthookproxy.h>
#include <interfaces/iactionsexporter.h>
#include <interfaces/ihavetabs.h>
#include "core.h"
#include "commonjobadder.h"
#include "xmlsettingsmanager.h"
#include "iconthemeengine.h"
#include "childactioneventfilter.h"
#include "tagsviewer.h"
#include "application.h"
#include "startupwizard.h"
#include "aboutdialog.h"
#include "toolbarguard.h"
#include "newtabmenumanager.h"
#include "tabmanager.h"
#include "coreinstanceobject.h"
#include "coreplugin2manager.h"
#include "entitymanager.h"

using namespace LeechCraft;
using namespace LeechCraft::Util;

LeechCraft::MainWindow::MainWindow (QWidget *parent, Qt::WFlags flags)
: QMainWindow (parent, flags)
, TrayIcon_ (0)
, IsShown_ (true)
, WasMaximized_ (false)
, IsQuitting_ (false)
, IsToolBarVisible_ (true)
{
	Guard_ = new ToolbarGuard (this);
	setUpdatesEnabled (false);

	hide ();

	Core::Instance ().GetCoreInstanceObject ()->
			GetCorePluginManager ()->RegisterHookable (this);

	InitializeInterface ();

	connect (qApp,
			SIGNAL (aboutToQuit ()),
			this,
			SLOT (handleQuit ()));

	connect (Core::Instance ().GetNewTabMenuManager (),
			SIGNAL (restoreTabActionAdded (QAction*)),
			this,
			SLOT (handleRestoreActionAdded (QAction*)));

	setUpdatesEnabled (true);

	if (!qobject_cast<Application*> (qApp)->GetVarMap ().count ("minimized"))
	{
		show ();
		activateWindow ();
		raise ();
	}
	else
	{
		IsShown_ = false;
		hide ();
	}

	WasMaximized_ = isMaximized ();
	Ui_.ActionFullscreenMode_->setChecked (isFullScreen ());
	QTimer::singleShot (700,
			this,
			SLOT (doDelayedInit ()));

	FullScreenShortcut_ = new QShortcut (QKeySequence (tr ("F11", "FullScreen")), this);
	FullScreenShortcut_->setContext (Qt::WidgetWithChildrenShortcut);
	connect (FullScreenShortcut_,
			SIGNAL (activated ()),
			this,
			SLOT (handleShortcutFullscreenMode ()));

	CloseTabShortcut_ = new QShortcut (QString ("Ctrl+W"),
			this,
			SLOT (handleCloseCurrentTab ()),
			0,
			Qt::ApplicationShortcut);

	Ui_.ActionShowToolBar_->setChecked (IsToolBarVisible_);
}

void LeechCraft::MainWindow::handleShortcutFullscreenMode ()
{
	on_ActionFullscreenMode__triggered (!isFullScreen ());
}

LeechCraft::MainWindow::~MainWindow ()
{
}

SeparateTabWidget* LeechCraft::MainWindow::GetTabWidget () const
{
	return Ui_.MainTabWidget_;
}

QSplitter* LeechCraft::MainWindow::GetMainSplitter () const
{
	return Ui_.MainSplitter_;
}

void LeechCraft::MainWindow::SetAdditionalTitle (const QString& title)
{
	if (title.isEmpty ())
		setWindowTitle ("LeechCraft");
	else
		setWindowTitle (QString ("%1 - LeechCraft").arg (title));
}

LeechCraft::ToolbarGuard* LeechCraft::MainWindow::GetGuard () const
{
	return Guard_;
}

QMenu* LeechCraft::MainWindow::GetMainMenu () const
{
	return Ui_.ActionMenu_->menu ();
}

void LeechCraft::MainWindow::HideMainMenu ()
{
	Ui_.ActionMenu_->setVisible (false);
}

QWidget* LeechCraft::MainWindow::GetDockListWidget (Qt::DockWidgetArea area) const
{
	switch (area)
	{
	case Qt::LeftDockWidgetArea:
		return Ui_.LeftDockButtons_;
	case Qt::RightDockWidgetArea:
		return Ui_.RightDockButtons_;
	default:
		return 0;
	}
}

void LeechCraft::MainWindow::ToggleViewActionVisiblity (QDockWidget *widget, bool visible)
{
	Util::DefaultHookProxy_ptr proxy (new Util::DefaultHookProxy);
	emit hookDockWidgetActionVisToggled (proxy, widget, visible);
	if (proxy->IsCancelled ())
		return;

	QAction *act = widget->toggleViewAction ();

	if (!visible)
		MenuView_->removeAction (act);
	else
		MenuView_->insertAction (MenuView_->actions ().first (), act);
}

void LeechCraft::MainWindow::AddMenus (const QMap<QString, QList<QAction*>>& menus)
{
	Q_FOREACH (const QString& menuName, menus.keys ())
	{
		QMenu *toInsert = 0;
		if (menuName == "view")
			toInsert = MenuView_;
		else if (menuName == "tools")
			toInsert = MenuTools_;
		else
		{
			const QList<QAction*>& actions = Ui_.ActionMenu_->menu ()->actions ();
			Q_FOREACH (QAction *action, actions)
				if (action->menu () &&
					action->text () == menuName)
				{
					toInsert = action->menu ();
					break;
				}
		}

		if (toInsert)
			toInsert->insertActions (toInsert->actions ().value (0, 0),
					menus [menuName]);
		else
		{
			QMenu *menu = new QMenu (menuName, Ui_.ActionMenu_->menu ());
			menu->addActions (menus [menuName]);
			Ui_.ActionMenu_->menu ()->insertMenu (MenuTools_->menuAction (), menu);
		}

		IconThemeEngine::Instance ().UpdateIconSet (menus [menuName]);
	}
}

void LeechCraft::MainWindow::RemoveMenus (const QMap<QString, QList<QAction*>>& menus)
{
	if (IsQuitting_)
		return;

	Q_FOREACH (const QString& menuName, menus.keys ())
	{
		QMenu *toRemove = 0;
		if (menuName == "view")
			toRemove = MenuView_;
		else if (menuName == "tools")
			toRemove = MenuTools_;

		if (toRemove)
			Q_FOREACH (QAction *action, menus [menuName])
				toRemove->removeAction (action);
		else
		{
			auto menu = Ui_.ActionMenu_->menu ();
			Q_FOREACH (QAction *action, menu->actions ())
				if (action->text () == menuName)
				{
					menu->removeAction (action);
					break;
				}
		}
	}
}

void LeechCraft::MainWindow::catchError (QString message)
{
	Entity e = Util::MakeEntity ("LeechCraft",
			QString (),
			AutoAccept | OnlyHandle,
			"x-leechcraft/notification");
	e.Additional_ ["Text"] = message;
	e.Additional_ ["Priority"] = PWarning_;
	Core::Instance ().handleGotEntity (e);
}

void LeechCraft::MainWindow::closeEvent (QCloseEvent *e)
{
	e->ignore ();
	if (XmlSettingsManager::Instance ()->
			property ("ExitOnClose").toBool ())
		on_ActionQuit__triggered ();
	else
	{
		hide ();
		IsShown_ = false;
	}
}

void LeechCraft::MainWindow::InitializeInterface ()
{
	installEventFilter (new ChildActionEventFilter (this));

	Ui_.setupUi (this);

	Ui_.MainTabWidget_->setObjectName ("org_LeechCraft_MainWindow_CentralTabWidget");
	Ui_.MainTabWidget_->SetTabsClosable (true);
	connect (Ui_.ActionAboutQt_,
			SIGNAL (triggered ()),
			qApp,
			SLOT (aboutQt ()));

	MenuView_ = new QMenu (tr ("View"), this);
	MenuView_->addSeparator ();
	MenuView_->addAction (Ui_.ActionShowStatusBar_);
	MenuView_->addAction (Ui_.ActionFullscreenMode_);
	MenuTools_ = new QMenu (tr ("Tools"), this);

#ifdef Q_OS_MAC
	Ui_.ActionFullscreenMode_->setVisible (false);
#endif

	Ui_.ActionAddTask_->setProperty ("ActionIcon", "list-add");
	Ui_.ActionCloseTab_->setProperty ("ActionIcon", "tab-close");
	Ui_.ActionSettings_->setProperty ("ActionIcon", "preferences-system");
	Ui_.ActionSettings_->setMenuRole (QAction::PreferencesRole);
	Ui_.ActionAboutLeechCraft_->setProperty ("ActionIcon", "help-about");
	Ui_.ActionAboutLeechCraft_->setMenuRole (QAction::AboutRole);
	Ui_.ActionAboutQt_->setIcon (qApp->style ()->
			standardIcon (QStyle::SP_MessageBoxQuestion).pixmap (32, 32));
	Ui_.ActionAboutQt_->setMenuRole (QAction::AboutQtRole);
	Ui_.ActionQuit_->setProperty ("ActionIcon", "application-exit");
	Ui_.ActionQuit_->setMenuRole (QAction::QuitRole);
	Ui_.ActionFullscreenMode_->setProperty ("ActionIcon", "view-fullscreen");
	Ui_.ActionFullscreenMode_->setParent (this);

	Ui_.MainTabWidget_->AddAction2TabBar (Ui_.ActionCloseTab_);
	connect (Ui_.MainTabWidget_,
			SIGNAL (newTabMenuRequested ()),
			this,
			SLOT (handleNewTabMenuRequested ()));

	XmlSettingsManager::Instance ()->RegisterObject ("ToolButtonStyle",
			this, "handleToolButtonStyleChanged");
	handleToolButtonStyleChanged ();
	XmlSettingsManager::Instance ()->RegisterObject ("ToolBarVisibilityManipulation",
			this, "handleToolBarManipulationChanged");
	handleToolBarManipulationChanged ();

	QMenu *menu = new QMenu (this);
	menu->addAction (Ui_.ActionAddTask_);
	menu->addSeparator ();
	menu->addMenu (MenuTools_);
	menu->addMenu (MenuView_);
	menu->addSeparator ();
	menu->addAction (Ui_.ActionSettings_);
	menu->addSeparator ();
	menu->addAction (Ui_.ActionAboutLeechCraft_);
	menu->addSeparator ();
	menu->addAction (Ui_.ActionRestart_);
	menu->addAction (Ui_.ActionQuit_);
	Ui_.ActionMenu_->setMenu (menu);

	SetStatusBar ();
	ReadSettings ();

	Ui_.MainTabWidget_->AddAction2TabBarLayout (QTabBar::LeftSide, Ui_.ActionMenu_);
}

void LeechCraft::MainWindow::SetStatusBar ()
{
	const int height = statusBar ()->sizeHint ().height ();

	QLBar_ = new QToolBar ();
	QLBar_->setIconSize (QSize (height - 1, height - 1));
	QLBar_->setMaximumHeight (height - 1);
	statusBar ()->addPermanentWidget (QLBar_);
}

void LeechCraft::MainWindow::ReadSettings ()
{
	QSettings settings ("Deviant", "Leechcraft");
	settings.beginGroup ("geometry");
	resize (settings.value ("size", QSize  (1150, 800)).toSize ());
	move   (settings.value ("pos",  QPoint (10, 10)).toPoint ());
	WasMaximized_ = settings.value ("maximized").toBool ();
	WasMaximized_ ? showMaximized () : showNormal ();
	settings.endGroup ();
	settings.beginGroup ("Window");
	Ui_.ActionShowStatusBar_->setChecked (settings.value ("StatusBarEnabled", true).toBool ());
	on_ActionShowStatusBar__triggered ();
	settings.endGroup ();
}

void LeechCraft::MainWindow::WriteSettings ()
{
	QSettings settings ("Deviant", "Leechcraft");
	settings.beginGroup ("geometry");
	settings.setValue ("size", size ());
	settings.setValue ("pos",  pos ());
	settings.setValue ("maximized", isMaximized ());
	settings.endGroup ();
	settings.beginGroup ("Window");
	settings.setValue ("StatusBarEnabled",
			Ui_.ActionShowStatusBar_->isChecked ());
	settings.endGroup ();
}

void LeechCraft::MainWindow::on_ActionAddTask__triggered ()
{
	CommonJobAdder adder (this);
	if (adder.exec () != QDialog::Accepted)
		return;

	QString name = adder.GetString ();
	if (!name.isEmpty ())
		Core::Instance ().TryToAddJob (name);
}

void LeechCraft::MainWindow::on_ActionCloseTab__triggered ()
{
	Core::Instance ().GetTabManager ()->
			remove (Ui_.MainTabWidget_->GetLastContextMenuTab ());
}

void MainWindow::handleCloseCurrentTab ()
{
	Core::Instance ().GetTabManager ()->
			remove (Ui_.MainTabWidget_->CurrentIndex ());
}

void LeechCraft::MainWindow::on_ActionSettings__triggered ()
{
	Core::Instance ().GetCoreInstanceObject ()->TabOpenRequested ("org.LeechCraft.SettingsPane");
}

void LeechCraft::MainWindow::on_ActionAboutLeechCraft__triggered ()
{
	AboutDialog *dia = new AboutDialog (this);
	dia->setAttribute (Qt::WA_DeleteOnClose);
	dia->show ();
}

void LeechCraft::MainWindow::on_ActionRestart__triggered()
{
	if (QMessageBox::question (this,
				"LeechCraft",
				tr ("Do you really want to restart?"),
				QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
		return;

	static_cast<Application*> (qApp)->InitiateRestart ();
	QTimer::singleShot (1000,
			qApp,
			SLOT (quit ()));
}

void LeechCraft::MainWindow::on_ActionQuit__triggered ()
{
	if (XmlSettingsManager::Instance ()->property ("ConfirmQuit").toBool ())
	{
		QMessageBox mbox (QMessageBox::Question,
				"LeechCraft",
				tr ("Do you really want to quit?"),
				QMessageBox::Yes | QMessageBox::No,
				this);
		mbox.setDefaultButton (QMessageBox::No);

		QPushButton always (tr ("Always"));
		mbox.addButton (&always, QMessageBox::AcceptRole);

		if (mbox.exec () == QMessageBox::No)
			return;
		else if (mbox.clickedButton () == &always)
			XmlSettingsManager::Instance ()->setProperty ("ConfirmQuit", false);
	}

	setEnabled (false);
	qApp->quit ();
}

void LeechCraft::MainWindow::on_ActionShowStatusBar__triggered ()
{
	statusBar ()->setVisible (Ui_.ActionShowStatusBar_->isChecked ());
}

void LeechCraft::MainWindow::on_ActionMenu__triggered ()
{
	QMenu *menu = Ui_.ActionMenu_->menu ();
	menu->exec (QCursor::pos ());
}

void LeechCraft::MainWindow::handleQuit ()
{
	WriteSettings ();
	hide ();

	IsQuitting_ = true;

	disconnect (Ui_.MainTabWidget_,
				0,
				0,
				0);

	TrayIcon_->hide ();
	delete TrayIcon_;
}

void LeechCraft::MainWindow::on_ActionFullscreenMode__triggered (bool full)
{
	if (full)
	{
		WasMaximized_ = isMaximized ();
		ShowMenuAndBar (false);
		showFullScreen ();
	}
	else if (WasMaximized_)
	{
		ShowMenuAndBar (true);
		showMaximized ();
		// Because shit happens on X11 otherwise
		QTimer::singleShot (200,
				this,
				SLOT (showMaximized ()));
	}
	else
	{
		ShowMenuAndBar (true);
		showNormal ();
	}
}

void LeechCraft::MainWindow::on_MainTabWidget__currentChanged (int index)
{
	QToolBar *bar = Core::Instance ().GetToolBar (index);
	GetGuard ()->AddToolbar (bar);
	if (Ui_.MainTabWidget_->WidgetCount () > 0 &&
			bar)
		bar->setVisible (IsToolBarVisible_);
}

void MainWindow::on_ActionShowToolBar__triggered (bool visible)
{
	IsToolBarVisible_ = visible;
	QToolBar *bar = Core::Instance ().GetToolBar (Ui_.MainTabWidget_->CurrentIndex ());
	if (bar)
		bar->setVisible (IsToolBarVisible_);
}

namespace
{
	Qt::ToolButtonStyle GetToolButtonStyle ()
	{
		QString style = XmlSettingsManager::Instance ()->
			property ("ToolButtonStyle").toString ();
		if (style == "iconOnly")
			return Qt::ToolButtonIconOnly;
		else if (style == "textOnly")
			return Qt::ToolButtonTextOnly;
		else if (style == "textBesideIcon")
			return Qt::ToolButtonTextBesideIcon;
		else
			return Qt::ToolButtonTextUnderIcon;
	}
};

void LeechCraft::MainWindow::handleToolButtonStyleChanged ()
{
	setToolButtonStyle (GetToolButtonStyle ());
}

void MainWindow::handleToolBarManipulationChanged ()
{
	if (XmlSettingsManager::Instance ()->property ("ToolBarVisibilityManipulation").toBool())
		MenuView_->insertAction (0, Ui_.ActionShowToolBar_);
	else
		MenuView_->removeAction (Ui_.ActionShowToolBar_);
}

void MainWindow::handleShowTrayIconChanged ()
{
	const bool isVisible = XmlSettingsManager::Instance ()->
			property ("ShowTrayIcon").toBool ();

	Util::DefaultHookProxy_ptr proxy (new Util::DefaultHookProxy);
	emit hookTrayIconVisibilityChanged (proxy, TrayIcon_, isVisible);
	if (proxy->IsCancelled ())
		return;

	TrayIcon_->setVisible (isVisible);
}

void LeechCraft::MainWindow::handleNewTabMenuRequested ()
{
	QMenu *ntmenu = Core::Instance ()
			.GetNewTabMenuManager ()->GetNewTabMenu ();
	ntmenu->popup (QCursor::pos ());
}

void MainWindow::handleRestoreActionAdded (QAction *act)
{
	Ui_.MainTabWidget_->InsertAction2TabBar (Ui_.ActionCloseTab_, act);
}

void LeechCraft::MainWindow::showHideMain ()
{
	IsShown_ = 1 - IsShown_;
	if (IsShown_)
	{
		show ();
		activateWindow ();
		raise ();
	}
	else
		hide ();
}

void LeechCraft::MainWindow::handleTrayIconActivated (QSystemTrayIcon::ActivationReason reason)
{
	switch (reason)
	{
		case QSystemTrayIcon::Context:
		case QSystemTrayIcon::Unknown:
			return;
		case QSystemTrayIcon::DoubleClick:
		case QSystemTrayIcon::Trigger:
		case QSystemTrayIcon::MiddleClick:
			showHideMain ();
			return;
	}
}

void LeechCraft::MainWindow::doDelayedInit ()
{
	FillQuickLaunch ();
	FillTray ();
	FillToolMenu ();
	InitializeShortcuts ();

	setAcceptDrops (true);

	new StartupWizard (this);
}

void LeechCraft::MainWindow::FillQuickLaunch ()
{
	Util::DefaultHookProxy_ptr proxy (new Util::DefaultHookProxy);
	emit hookGonnaFillMenu (proxy);
	if (proxy->IsCancelled ())
		return;

	const auto& exporters = Core::Instance ().GetPluginManager ()->GetAllCastableTo<IActionsExporter*> ();
	Q_FOREACH (auto exp, exporters)
	{
		const auto& map = exp->GetMenuActions ();
		if (!map.isEmpty ())
			AddMenus (map);
	}

	proxy.reset (new Util::DefaultHookProxy);
	emit hookGonnaFillQuickLaunch (proxy);
	if (proxy->IsCancelled ())
		return;

	Q_FOREACH (auto exp, exporters)
	{
		const auto& actions = exp->GetActions (ActionsEmbedPlace::QuickLaunch);
		if (actions.isEmpty ())
			continue;

		IconThemeEngine::Instance ().UpdateIconSet (actions);

		QLBar_->addSeparator ();
		QLBar_->addActions (actions);
	}
}

void LeechCraft::MainWindow::FillTray ()
{
	QMenu *iconMenu = new QMenu (this);
	QMenu *menu = iconMenu->addMenu (tr ("LeechCraft menu"));
	menu->addAction (Ui_.ActionAddTask_);
	menu->addMenu (MenuView_);
	menu->addMenu (MenuTools_);
	iconMenu->addSeparator ();

	const auto& trayMenus = Core::Instance ().GetPluginManager ()->
			GetAllCastableTo<IActionsExporter*> ();
	Q_FOREACH (auto o, trayMenus)
	{
		const auto& actions = o->GetActions (ActionsEmbedPlace::TrayMenu);
		IconThemeEngine::Instance ().UpdateIconSet (actions);
		iconMenu->addActions (actions);
		if (actions.size ())
			iconMenu->addSeparator ();
	}

	iconMenu->addAction (Ui_.ActionQuit_);

	TrayIcon_ = new QSystemTrayIcon (QIcon (":/resources/images/leechcraft.svg"), this);
	handleShowTrayIconChanged ();
	TrayIcon_->setContextMenu (iconMenu);
	connect (TrayIcon_,
			SIGNAL (activated (QSystemTrayIcon::ActivationReason)),
			this,
			SLOT (handleTrayIconActivated (QSystemTrayIcon::ActivationReason)));

	emit hookTrayIconCreated (Util::DefaultHookProxy_ptr (new Util::DefaultHookProxy), TrayIcon_);
	XmlSettingsManager::Instance ()->RegisterObject ("ShowTrayIcon",
			this, "handleShowTrayIconChanged");
	handleShowTrayIconChanged ();
}

void LeechCraft::MainWindow::FillToolMenu ()
{
	Q_FOREACH (IActionsExporter *e,
			Core::Instance ().GetPluginManager ()->
				GetAllCastableTo<IActionsExporter*> ())
	{
		const auto& acts = e->GetActions (ActionsEmbedPlace::ToolsMenu);
		IconThemeEngine::Instance ().UpdateIconSet (acts);
		MenuTools_->addActions (acts);
		if (acts.size ())
			MenuTools_->addSeparator ();
	}

	QMenu *ntm = Core::Instance ()
		.GetNewTabMenuManager ()->GetNewTabMenu ();
	Ui_.MainTabWidget_->SetAddTabButtonContextMenu (ntm);

	QMenu *atm = Core::Instance ()
		.GetNewTabMenuManager ()->GetAdditionalMenu ();

	int i = 0;
	Q_FOREACH (QAction *act, atm->actions ())
		Ui_.MainTabWidget_->InsertAction2TabBar (i++, act);

	on_MainTabWidget__currentChanged (0);
}

void LeechCraft::MainWindow::InitializeShortcuts ()
{
#ifndef Q_OS_MAC
	const auto sysModifier = Qt::CTRL;
#else
	const auto sysModifier = Qt::ALT;
#endif

	connect (new QShortcut (QKeySequence ("Ctrl+["), this),
			SIGNAL (activated ()),
			Core::Instance ().GetTabManager (),
			SLOT (rotateLeft ()));
	connect (new QShortcut (QKeySequence ("Ctrl+]"), this),
			SIGNAL (activated ()),
			Core::Instance ().GetTabManager (),
			SLOT (rotateRight ()));
	connect (new QShortcut (QKeySequence (sysModifier + Qt::Key_Tab), this),
			SIGNAL (activated ()),
			Core::Instance ().GetTabManager (),
			SLOT (rotateRight ()));
	connect (new QShortcut (QKeySequence (sysModifier + Qt::SHIFT + Qt::Key_Tab), this),
			SIGNAL (activated ()),
			Core::Instance ().GetTabManager (),
			SLOT (rotateLeft ()));
	connect (new QShortcut (QKeySequence ("Ctrl+PgUp"), this),
			SIGNAL (activated ()),
			Core::Instance ().GetTabManager (),
			SLOT (rotateLeft ()));
	connect (new QShortcut (QKeySequence ("Ctrl+PgDown"), this),
			SIGNAL (activated ()),
			Core::Instance ().GetTabManager (),
			SLOT (rotateRight ()));
	connect (new QShortcut (QKeySequence (Qt::CTRL + Qt::Key_T), this),
			SIGNAL (activated ()),
			Ui_.MainTabWidget_,
			SLOT (handleNewTabShortcutActivated ()));
	connect (new QShortcut (QKeySequence (sysModifier + Qt::Key_Space), this),
			SIGNAL (activated ()),
			Ui_.MainTabWidget_,
			SLOT (setPreviousTab ()));

	for (int i = 0; i < 10; ++i)
	{
		QString seqStr = QString ("Ctrl+\\, %1").arg (i);
		QShortcut *sc = new QShortcut (QKeySequence (seqStr), this);
		sc->setProperty ("TabNumber", i);
		connect (sc,
				SIGNAL (activated ()),
				Core::Instance ().GetTabManager (),
				SLOT (navigateToTabNumber ()));
	}
}

void LeechCraft::MainWindow::ShowMenuAndBar (bool show)
{
	if (XmlSettingsManager::Instance ()->property ("ToolBarVisibilityManipulation").toBool ())
		Ui_.ActionFullscreenMode_->setChecked (!show);
}

void LeechCraft::MainWindow::keyPressEvent (QKeyEvent *e)
{
	int index = (e->key () & ~Qt::CTRL) - Qt::Key_0;
	if (index == 0)
		index = 10;
	--index;
	if (index >= 0 && index < std::min (10, Ui_.MainTabWidget_->WidgetCount ()))
		Ui_.MainTabWidget_->setCurrentTab (index);
}

void LeechCraft::MainWindow::keyReleaseEvent (QKeyEvent *e)
{
	if (e->key () == Qt::Key_Alt &&
			XmlSettingsManager::Instance ()->property ("ToolBarVisibilityManipulation").toBool ())
	{
		on_ActionShowToolBar__triggered (!IsToolBarVisible_);
		Ui_.ActionShowToolBar_->setChecked (IsToolBarVisible_);
	}
}

void MainWindow::dragEnterEvent (QDragEnterEvent *event)
{
	auto mimeData = event->mimeData ();
	for (const QString& format : mimeData->formats ())
	{
		const Entity& e = Util::MakeEntity (mimeData->data (format),
				QString (),
				FromUserInitiated,
				format);

		if (EntityManager ().CouldHandle (e))
		{
			event->acceptProposedAction ();
			return;
		}
	}

	QMainWindow::dragEnterEvent (event);
}

void MainWindow::dropEvent (QDropEvent *event)
{
	auto mimeData = event->mimeData ();
	Q_FOREACH (const QString& format, mimeData->formats ())
	{
		const Entity& e = Util::MakeEntity (mimeData->data (format),
				QString (),
				FromUserInitiated,
				format);

		if (EntityManager ().HandleEntity (e))
		{
			event->acceptProposedAction ();
			break;
		}
	}

	QWidget::dropEvent (event);
}

