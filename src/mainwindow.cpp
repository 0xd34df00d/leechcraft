/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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
#include <QModelIndex>
#include <QChildEvent>
#include <QToolButton>
#include <QStandardItemModel>
#include <QCursor>
#include <QCheckBox>
#include <QShortcut>
#include <QClipboard>
#include <QMenu>
#include <QSplashScreen>
#include <QBitmap>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <util/util.h>
#include <interfaces/iactionsexporter.h>
#include <interfaces/ihavetabs.h>
#include "view.h"
#include "core.h"
#include "commonjobadder.h"
#include "xmlsettingsmanager.h"
#include "fancypopupmanager.h"
#include "skinengine.h"
#include "childactioneventfilter.h"
#include "logtoolbox.h"
#include "graphwidget.h"
#include "shortcutmanager.h"
#include "tagsviewer.h"
#include "application.h"
#include "startupwizard.h"
#include "aboutdialog.h"
#include "toolbarguard.h"
#include "newtabmenumanager.h"
#include "tabmanager.h"
#include "coreinstanceobject.h"

using namespace LeechCraft;
using namespace LeechCraft::Util;

LeechCraft::MainWindow::MainWindow (QWidget *parent, Qt::WFlags flags)
: QMainWindow (parent, flags)
, TrayIcon_ (0)
, IsShown_ (true)
, WasMaximized_ (false)
, DefaultSystemStyleName_ (QApplication::style ()->objectName ())
, IsQuitting_ (false)
, Splash_ (new QSplashScreen (QPixmap (":/resources/images/splashscreen.png"),
		Qt::SplashScreen))
{
	Guard_ = new ToolbarGuard (this);
	setUpdatesEnabled (false);

	hide ();
	Splash_->setMask (QPixmap (":/resources/images/splashscreen.png").mask ());
	Splash_->show ();
	Splash_->setUpdatesEnabled (true);
	Splash_->showMessage (tr ("Initializing LeechCraft..."), Qt::AlignLeft | Qt::AlignBottom);
	QApplication::processEvents ();
	
	Core::Instance ();

	InitializeInterface ();
	hide ();

	connect (qApp,
			SIGNAL (aboutToQuit ()),
			this,
			SLOT (handleQuit ()));

	connect (Core::Instance ().GetPluginManager (),
			SIGNAL (loadProgress (const QString&)),
			this,
			SLOT (handleLoadProgress (const QString&)));
	connect (&Core::Instance (),
			SIGNAL (log (const QString&)),
			LogToolBox_,
			SLOT (log (const QString&)));

	Core::Instance ().SetReallyMainWindow (this);
	Core::Instance ().DelayedInit ();
	
	Splash_->showMessage (tr ("Finalizing..."), Qt::AlignLeft | Qt::AlignBottom);

	connect (Core::Instance ().GetNewTabMenuManager (),
			SIGNAL (restoreTabActionAdded (QAction*)),
			this,
			SLOT (handleRestoreActionAdded (QAction*)));

	QTimer *speedUpd = new QTimer (this);
	speedUpd->setInterval (1000);
	connect (speedUpd,
			SIGNAL (timeout ()),
			this,
			SLOT (updateSpeedIndicators ()));
	connect (speedUpd,
			SIGNAL (timeout ()),
			this,
			SLOT (updateClock ()));
	speedUpd->start ();
	qApp->setQuitOnLastWindowClosed (false);

	updateIconSet ();

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
	
	Splash_->finish (this);

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

const IShortcutProxy* LeechCraft::MainWindow::GetShortcutProxy () const
{
	return ShortcutManager_;
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

LeechCraft::FancyPopupManager* LeechCraft::MainWindow::GetFancyPopupManager () const
{
	return FancyPopupManager_;
}

void LeechCraft::MainWindow::AddMenus (const QMap<QString, QList<QAction*> >& menus)
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
			QMenu *menu = new QMenu (menuName);
			menu->addActions (menus [menuName]);
			Ui_.ActionMenu_->menu ()->insertMenu (MenuTools_->menuAction (), menu);
		}
	}
}

void LeechCraft::MainWindow::RemoveMenus (const QMap<QString, QList<QAction*> >& menus)
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
			Q_FOREACH (QAction *action, Ui_.ActionMenu_->menu ()->actions ())
				if (action->text () == menuName)
				{
					Ui_.ActionMenu_->menu ()->removeAction (action);
					break;
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
	MenuView_->addAction (Ui_.ActionShowStatusBar_);
	MenuView_->addAction (Ui_.ActionFullscreenMode_);
	MenuTools_ = new QMenu (tr ("Tools"), this);

	Ui_.ActionAddTask_->setProperty ("ActionIcon", "addjob");
	Ui_.ActionCloseTab_->setProperty ("ActionIcon", "closetab");
	Ui_.ActionSettings_->setProperty ("ActionIcon", "settings");
	Ui_.ActionAboutLeechCraft_->setProperty ("ActionIcon", "about");
	Ui_.ActionAboutQt_->setIcon (qApp->style ()->
			standardIcon (QStyle::SP_MessageBoxQuestion).pixmap (32, 32));
	Ui_.ActionQuit_->setProperty ("ActionIcon", "exit");
	Ui_.ActionLogger_->setProperty ("ActionIcon", "logger");
	Ui_.ActionFullscreenMode_->setProperty ("ActionIcon", "fullscreen");
	Ui_.ActionFullscreenMode_->setParent (this);
	Ui_.ActionShowStatusBar_->setProperty ("ActionIcon", "showstatusbar");

	Ui_.MainTabWidget_->AddAction2TabBar (Ui_.ActionCloseTab_);
	connect (Ui_.MainTabWidget_,
			SIGNAL (newTabMenuRequested ()),
			this,
			SLOT (handleNewTabMenuRequested ()));

	XmlSettingsManager::Instance ()->RegisterObject ("AppQStyle",
			this, "handleAppStyle");
	handleAppStyle ();

	XmlSettingsManager::Instance ()->RegisterObject ("Language",
			this, "handleLanguage");
	XmlSettingsManager::Instance ()->RegisterObject ("ToolButtonStyle",
			this, "handleToolButtonStyleChanged");
	handleToolButtonStyleChanged ();
	XmlSettingsManager::Instance ()->RegisterObject ("IconSize",
			this, "handleIconSize");
	handleIconSize ();

	LanguageOnLoad_ = XmlSettingsManager::Instance ()->property ("Language").toString ();

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
	menu->addAction (Ui_.ActionQuit_);
	Ui_.ActionMenu_->setMenu (menu);

	XmlSettingsManager::Instance ()->RegisterObject ("IconSet", this, "updateIconSet");

	ShortcutManager_ = new ShortcutManager (this);
	Core::Instance ().GetCoreInstanceObject ()->GetSettingsDialog ()->SetCustomWidget ("ShortcutManager", ShortcutManager_);

	SetStatusBar ();
	ReadSettings ();

	LogToolBox_ = new LogToolBox (this);

	Ui_.MainTabWidget_->AddAction2TabBarLayout (QTabBar::LeftSide, Ui_.ActionMenu_);
}


void LeechCraft::MainWindow::SetStatusBar ()
{
	QFontMetrics fm = fontMetrics ();
	int minSize = fm.width (Util::MakePrettySize (999) + tr ("/s	"));

	DownloadSpeed_ = new QLabel;
	DownloadSpeed_->setText (Util::MakePrettySize (0) + tr ("/s"));
	DownloadSpeed_->setMinimumWidth (minSize);
	DownloadSpeed_->setAlignment (Qt::AlignRight);
	UploadSpeed_ = new QLabel;
	UploadSpeed_->setText (Util::MakePrettySize (0) + tr ("/s"));
	UploadSpeed_->setMinimumWidth (minSize);
	UploadSpeed_->setAlignment (Qt::AlignRight);
	QString current = QTime::currentTime ().toString ();
	Clock_ = new QLabel;
	Clock_->setMinimumWidth (fm.width (current + "___"));
	Clock_->setText (current);
	Clock_->setAlignment (Qt::AlignRight);

	SpeedGraph_ = new GraphWidget (Qt::green, Qt::red);
	SpeedGraph_->setMinimumWidth (250);

	statusBar ()->addPermanentWidget (SpeedGraph_);
	statusBar ()->addPermanentWidget (DownloadSpeed_);
	statusBar ()->addPermanentWidget (UploadSpeed_);
	statusBar ()->addPermanentWidget (Clock_);
	if (!isFullScreen ())
		Clock_->hide ();
}

void LeechCraft::MainWindow::ReadSettings ()
{
	QSettings settings ("Deviant", "Leechcraft");
	settings.beginGroup ("geometry");
	resize (settings.value ("size", QSize  (750, 550)).toSize ());
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

	Core::Instance ().Release ();

	TrayIcon_->hide ();
	delete TrayIcon_;
#ifdef QT_DEBUG
	qDebug () << "Releasing XmlSettingsManager";
#endif
	XmlSettingsManager::Instance ()->Release ();
#ifdef QT_DEBUG
	qDebug () << "Destroyed fine";
#endif
}

void LeechCraft::MainWindow::handleAppStyle ()
{
	QString style = XmlSettingsManager::Instance ()->
			property ("AppQStyle").toString ();

	if (style == "Default")
		style = DefaultSystemStyleName_;

	if (style.isEmpty ())
	{
#ifdef Q_WS_WIN
		style = "Plastique";
		XmlSettingsManager::Instance ()->
		setProperty ("AppQStyle", style);
#endif
	}

	QApplication::setStyle (style);
}

void LeechCraft::MainWindow::handleLanguage ()
{
	if (LanguageOnLoad_ == XmlSettingsManager::Instance ()->
			property ("Language").toString ())
		return;

	if (QMessageBox::question (this,
				"LeechCraft",
				tr ("This change requires restarting LeechCraft. "
					"Do you want to restart now?"),
				QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
		return;

	static_cast<Application*> (qApp)->InitiateRestart ();
}

void LeechCraft::MainWindow::on_ActionFullscreenMode__triggered (bool full)
{
	if (full)
	{
		WasMaximized_ = isMaximized ();
		ShowMenuAndBar (false);
		showFullScreen ();
		Clock_->show ();
	}
	else if (WasMaximized_)
	{
		ShowMenuAndBar (true);
		Clock_->hide ();
		showMaximized ();
		// Because shit happens on X11 otherwise
		QTimer::singleShot (200,
				this,
				SLOT (showMaximized ()));
	}
	else
	{
		ShowMenuAndBar (true);
		Clock_->hide ();
		showNormal ();
	}
}

void LeechCraft::MainWindow::on_ActionLogger__triggered ()
{
	LogToolBox_->show ();
}

void LeechCraft::MainWindow::on_MainTabWidget__currentChanged (int index)
{
	QToolBar *bar = Core::Instance ().GetToolBar (index);
	if (!bar)
		return;
	GetGuard ()->AddToolbar (bar);
	if (Ui_.MainTabWidget_->WidgetCount () > 0)
		bar->setVisible (!isFullScreen ());
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

void LeechCraft::MainWindow::handleIconSize ()
{
	int size = XmlSettingsManager::Instance ()->
		property ("IconSize").toInt ();
	if (size)
		setIconSize (QSize (size, size));
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

void LeechCraft::MainWindow::updateSpeedIndicators ()
{
	QPair<qint64, qint64> speeds = Core::Instance ().GetSpeeds ();

	QString down = Util::MakePrettySize (speeds.first) + tr ("/s");
	QString up = Util::MakePrettySize (speeds.second) + tr ("/s");
	DownloadSpeed_->setText (down);
	UploadSpeed_->setText (up);
	SpeedGraph_->PushSpeed (speeds.first, speeds.second);

	if (TrayIcon_)
		TrayIcon_->setToolTip (tr ("%1 down, %2 up")
				.arg (down)
				.arg (up));
}

void LeechCraft::MainWindow::updateClock ()
{
	Clock_->setText (QTime::currentTime ().toString ());
}

void LeechCraft::MainWindow::showHideMain ()
{
	IsShown_ = 1 - IsShown_;
	IsShown_ ? show () : hide ();
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

void LeechCraft::MainWindow::updateIconSet ()
{
	SkinEngine::Instance ().UpdateIconSet (findChildren<QAction*> ());
	SkinEngine::Instance ().UpdateIconSet (findChildren<QTabWidget*> ());
}

void LeechCraft::MainWindow::doDelayedInit ()
{
	QObjectList shortcuts = Core::Instance ().GetShortcuts ();
	for (QObjectList::const_iterator i = shortcuts.begin (),
			end = shortcuts.end (); i != end; ++i)
		ShortcutManager_->AddObject (*i);
	
	QList<IActionsExporter*> exporters = Core::Instance ()
			.GetPluginManager ()->GetAllCastableTo<IActionsExporter*> ();
	Q_FOREACH (IActionsExporter *exp, exporters)
	{
		QMap<QString, QList<QAction*> > map = exp->GetMenuActions ();
		if (!map.isEmpty ())
			AddMenus (map);
		
		QList<QAction*> actions = exp->GetActions (AEPQuickLaunch);
		if (actions.isEmpty ())
			continue;

		Q_FOREACH (QAction *action, actions)
			Ui_.MainTabWidget_->
					AddAction2TabBarLayout (QTabBar::RightSide, action);
		Ui_.MainTabWidget_->AddAction2TabBarLayout (QTabBar::RightSide,
						Util::CreateSeparator (this));
	}

	FillTray ();
	FillToolMenu ();
	InitializeShortcuts ();

	setAcceptDrops (true);

	new StartupWizard (this);
}

void LeechCraft::MainWindow::handleLoadProgress (const QString& str)
{
	Splash_->showMessage (str, Qt::AlignLeft | Qt::AlignBottom);
}

void LeechCraft::MainWindow::FillTray ()
{
	QMenu *iconMenu = new QMenu (this);
	QMenu *menu = iconMenu->addMenu (tr ("LeechCraft menu"));
	menu->addAction (Ui_.ActionAddTask_);
	menu->addMenu (MenuView_);
	menu->addMenu (MenuTools_);
	iconMenu->addSeparator ();

	QList<IActionsExporter*> trayMenus = Core::Instance ()
			.GetPluginManager ()->GetAllCastableTo<IActionsExporter*> ();
	Q_FOREACH (IActionsExporter *o, trayMenus)
	{
		QList<QAction*> actions = o->GetActions (AEPTrayMenu);
		iconMenu->addActions (actions);
		if (actions.size ())
			iconMenu->addSeparator ();
	}

	iconMenu->addAction (Ui_.ActionQuit_);

	TrayIcon_ = new QSystemTrayIcon (QIcon (":/resources/images/leechcraft.svg"), this);
	TrayIcon_->show ();
	FancyPopupManager_ = new FancyPopupManager (TrayIcon_, this);
	TrayIcon_->setContextMenu (iconMenu);
	connect (TrayIcon_,
			SIGNAL (activated (QSystemTrayIcon::ActivationReason)),
			this,
			SLOT (handleTrayIconActivated (QSystemTrayIcon::ActivationReason)));
}

void LeechCraft::MainWindow::FillToolMenu ()
{
	Q_FOREACH (IActionsExporter *e,
			Core::Instance ().GetPluginManager ()->
				GetAllCastableTo<IActionsExporter*> ())
	{
		QList<QAction*> acts = e->GetActions (AEPToolsMenu);

		Q_FOREACH (QAction *action, acts)
			MenuTools_->insertAction (Ui_.ActionLogger_, action);
		if (acts.size ())
			MenuTools_->insertSeparator (Ui_.ActionLogger_);
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
	connect (new QShortcut (QKeySequence ("Ctrl+["), this),
			SIGNAL (activated ()),
			Core::Instance ().GetTabManager (),
			SLOT (rotateLeft ()));
	connect (new QShortcut (QKeySequence ("Ctrl+]"), this),
			SIGNAL (activated ()),
			Core::Instance ().GetTabManager (),
			SLOT (rotateRight ()));
	connect (new QShortcut (QKeySequence (Qt::CTRL + Qt::Key_Tab), this),
			SIGNAL (activated ()),
			Core::Instance ().GetTabManager (),
			SLOT (rotateRight ()));
	connect (new QShortcut (QKeySequence (Qt::CTRL + Qt::SHIFT + Qt::Key_Tab), this),
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
	connect (new QShortcut (QKeySequence (Qt::CTRL + Qt::Key_Space), this),
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

namespace
{
	QVariant ClipboardToEcontent (QString selection)
	{
		QVariant econtent;
		if (selection=="")
			return econtent;
		if (QFile::exists (selection))
			econtent = QUrl::fromLocalFile (selection);
		else
		{
			QUrl url (selection);
			if (url.isValid ())
				econtent = url;
			else
				econtent = selection;
		}
		return econtent;
	}
};

void LeechCraft::MainWindow::ShowMenuAndBar (bool show)
{
	int cur = Ui_.MainTabWidget_->CurrentIndex ();
	if (Core::Instance ().GetToolBar (cur))
		Core::Instance ().GetToolBar (cur)->setVisible (show);
	Ui_.MainTabWidget_->SetToolBarVisible (show);
	Ui_.ActionFullscreenMode_->setChecked (!show);
}

void LeechCraft::MainWindow::keyPressEvent(QKeyEvent* e)
{
	int index = (e->key () & ~Qt::CTRL) - Qt::Key_0;
	if (index == 0)
		index = 10;
	--index;
	if (index >= 0 && index < std::min (10, Ui_.MainTabWidget_->WidgetCount ()))
		Ui_.MainTabWidget_->setCurrentIndex (index);
}
