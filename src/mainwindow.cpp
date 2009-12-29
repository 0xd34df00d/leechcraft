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

#include <iostream>
#include <algorithm>
#include <QtGui/QtGui>
#include <QModelIndex>
#include <QChildEvent>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <plugininterface/util.h>
#include <interfaces/itraymenu.h>
#include <interfaces/imenuembedder.h>
#include <interfaces/imultitabs.h>
#include "mainwindow.h"
#include "view.h"
#include "core.h"
#include "commonjobadder.h"
#include "xmlsettingsmanager.h"
#include "pluginmanagerdialog.h"
#include "fancypopupmanager.h"
#include "skinengine.h"
#include "childactioneventfilter.h"
#include "logtoolbox.h"
#include "settingssink.h"
#include "iconchooser.h"
#include "graphwidget.h"
#include "shortcutmanager.h"
#include "appstyler.h"
#include "tagsviewer.h"
#include "application.h"
#include "startupwizard.h"
#include "aboutdialog.h"
#include "toolbarguard.h"

using namespace LeechCraft;
using namespace LeechCraft::Util;

LeechCraft::MainWindow::MainWindow (QWidget *parent, Qt::WFlags flags)
: QMainWindow (parent, flags)
, TrayIcon_ (0)
, IsShown_ (true)
, WasMaximized_ (false)
, PluginsActionsBar_ (0)
{
	Guard_ = new ToolbarGuard (this);
	setUpdatesEnabled (false);
	InitializeInterface ();

	connect (qApp,
			SIGNAL (aboutToQuit ()),
			this,
			SLOT (handleQuit ()));

	connect (&Core::Instance (),
			SIGNAL (downloadFinished (const QString&)),
			this,
			SLOT (handleDownloadFinished (QString)));
	connect (&Core::Instance (),
			SIGNAL (log (const QString&)),
			LogToolBox_,
			SLOT (log (const QString&)));

	Core::Instance ().SetReallyMainWindow (this);
	Core::Instance ().DelayedInit ();

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

	Q_FOREACH (QList<QAction*> list, Core::Instance ().GetActions2Embed ())
	{
		Q_FOREACH (QAction *act, list)
			act->setParent (this);
		if (list.size ())
		{
            if (!PluginsActionsBar_)
            {
                PluginsActionsBar_ = new QToolBar (tr ("Actions"), this);
				Qt::ToolBarArea area;
				QSettings settings ("Deviant", "Leechcraft");
				settings.beginGroup ("Window");
				area = static_cast<Qt::ToolBarArea> (settings
						.value ("PluginsArea", 1).toInt ());
				addToolBar (area, PluginsActionsBar_);
				PluginsActionsBar_->setVisible (settings
						.value ("PluginsBarVisible", true).toBool ());
            }
			PluginsActionsBar_->addActions (list);
			PluginsActionsBar_->addSeparator ();
		}
	}

	updateIconSet ();

	setUpdatesEnabled (true);
	show ();

	WasMaximized_ = isMaximized ();
	Ui_.ActionFullscreenMode_->setChecked (isFullScreen ());
	QTimer::singleShot (700,
			this,
			SLOT (doDelayedInit ()));
}

LeechCraft::MainWindow::~MainWindow ()
{
}

TabWidget* LeechCraft::MainWindow::GetTabWidget () const
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
		setWindowTitle (tr ("LeechCraft"));
	else
		setWindowTitle (tr ("%1 - LeechCraft").arg (title));
}

LeechCraft::ToolbarGuard* LeechCraft::MainWindow::GetGuard () const
{
	return Guard_;
}

void LeechCraft::MainWindow::catchError (QString message)
{
	QMessageBox::critical (this,
			tr ("LeechCraft"),
			message);
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

	connect (Ui_.ActionAboutQt_,
			SIGNAL (triggered ()),
			qApp,
			SLOT (aboutQt ()));

	Ui_.ActionAddTask_->setProperty ("ActionIcon", "addjob");
	Ui_.ActionNewTab_->setProperty ("ActionIcon", "newtab");
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

	Ui_.MainTabWidget_->setTabIcon (0, QIcon (":/resources/images/leechcraft.svg"));
	Ui_.MainTabWidget_->AddAction2TabBar (Ui_.ActionNewTab_);
	Ui_.MainTabWidget_->AddAction2TabBar (Ui_.ActionCloseTab_);

	QToolBar *bar = new QToolBar ();
	bar->addAction (Ui_.ActionNewTab_);
	bar->addAction (Ui_.ActionCloseTab_);
	Ui_.MainTabWidget_->setCornerWidget (bar, Qt::TopRightCorner);

	XmlSettingsDialog_ = new XmlSettingsDialog ();
	XmlSettingsDialog_->RegisterObject (XmlSettingsManager::Instance (),
			"coresettings.xml");
	connect (XmlSettingsDialog_,
			SIGNAL (pushButtonClicked (const QString&)),
			&Core::Instance (),
			SLOT (handleSettingClicked (const QString&)));

	XmlSettingsDialog_->SetCustomWidget ("AppQStyle", new AppStyler (this));
	XmlSettingsDialog_->SetCustomWidget ("TagsViewer", new TagsViewer);

	XmlSettingsManager::Instance ()->RegisterObject ("Language",
			this, "handleLanguage");
	XmlSettingsManager::Instance ()->RegisterObject ("ToolButtonStyle",
			this, "handleToolButtonStyleChanged");
	handleToolButtonStyleChanged ();
	XmlSettingsManager::Instance ()->RegisterObject ("ShowMenuBarAsButton",
			this, "handleShowMenuBarAsButton");

	LanguageOnLoad_ = XmlSettingsManager::Instance ()->property ("Language").toString ();

	QMenu *menu = new QMenu (this);
	menu->addMenu (Ui_.MenuGeneral_);
	menu->addMenu (Ui_.MenuView_);
	menu->addMenu (Ui_.MenuTools_);
	menu->addMenu (Ui_.MenuHelp_);
	Ui_.ActionMenu_->setMenu (menu);

	IconChooser *ic = new IconChooser (SkinEngine::Instance ().ListIcons (),
			this);
	connect (ic,
			SIGNAL (requestNewIconSet ()),
			this,
			SLOT (updateIconSet ()));

	XmlSettingsDialog_->SetCustomWidget ("IconSet", ic);

	SettingsSink_ = new SettingsSink (tr ("LeechCraft"),
			XmlSettingsDialog_);
	ShortcutManager_ = new ShortcutManager (this);
	XmlSettingsDialog_->SetCustomWidget ("ShortcutManager", ShortcutManager_);

	SetStatusBar ();
	ReadSettings ();

	LogToolBox_ = new LogToolBox (this);
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
	settings.setValue ("PluginsArea",
			toolBarArea (PluginsActionsBar_));
	settings.setValue ("PluginsBarVisible",
			PluginsActionsBar_->isVisible ());
    settings.endGroup ();
}

void LeechCraft::MainWindow::on_ActionAddTask__triggered ()
{
	CommonJobAdder adder (this);
	if (adder.exec () != QDialog::Accepted)
		return;

	QString name = adder.GetString ();
	if (!name.isEmpty ())
		Core::Instance ().TryToAddJob (name, adder.GetWhere ());
}

void LeechCraft::MainWindow::on_ActionNewTab__triggered ()
{
	IMultiTabsWidget *imtw =
		qobject_cast<IMultiTabsWidget*> (GetTabWidget ()->currentWidget ());
	if (imtw)
		imtw->NewTabRequested ();
}

void LeechCraft::MainWindow::on_ActionCloseTab__triggered ()
{
	QAction *act = qobject_cast<QAction*> (sender ());
	int pos = -1;
	if (act && act->data ().canConvert<QPoint> ())
	{
		pos = Ui_.MainTabWidget_->TabAt (act->data ().value<QPoint> ());
		act->setData (QVariant ());
	}
	else
		pos = Ui_.MainTabWidget_->currentIndex ();
	Core::Instance ().GetTabContainer ()->remove (pos);
}

void LeechCraft::MainWindow::on_ActionSettings__triggered ()
{
	SettingsSink_->show ();
}

void LeechCraft::MainWindow::on_ActionAboutLeechCraft__triggered ()
{
	AboutDialog *dia = new AboutDialog (this);
	dia->setAttribute (Qt::WA_DeleteOnClose);
	dia->show ();
}

void LeechCraft::MainWindow::on_ActionQuit__triggered ()
{
	if (XmlSettingsManager::Instance ()->property ("ConfirmQuit").toBool () &&
			QMessageBox::question (this,
				tr ("LeechCraft"),
				tr ("Do you really want to quit?"),
				QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
		return;

	setEnabled (false);
	qApp->quit ();
}

void LeechCraft::MainWindow::on_ActionShowStatusBar__triggered ()
{
    statusBar ()->setVisible (Ui_.ActionShowStatusBar_->isChecked ());
}

void LeechCraft::MainWindow::handleQuit ()
{
	WriteSettings ();
	Core::Instance ().Release ();

	TrayIcon_->hide ();
	delete TrayIcon_;
#ifdef QT_DEBUG
	qDebug () << "Releasing XmlSettingsManager";
#endif
	delete XmlSettingsDialog_;
	delete SettingsSink_;
	XmlSettingsManager::Instance ()->Release ();
#ifdef QT_DEBUG
	qDebug () << "Destroyed fine";
#endif
}

void LeechCraft::MainWindow::handleLanguage ()
{
	if (LanguageOnLoad_ == XmlSettingsManager::Instance ()->
			property ("Language").toString ())
		return;

	if (QMessageBox::question (this,
				tr ("LeechCraft"),
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
		showFullScreen ();
		Clock_->show ();
	}
	else if (WasMaximized_)
	{
		Clock_->hide ();
		showMaximized ();
		// Because shit happens on X11 otherwise
		QTimer::singleShot (200,
				this,
				SLOT (showMaximized ()));
	}
	else
	{
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
	GetGuard ()->AddToolbar (bar);
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

void LeechCraft::MainWindow::handleShowMenuBarAsButton ()
{
	bool asButton = XmlSettingsManager::Instance ()->
		property ("ShowMenuBarAsButton").toBool ();
	if (asButton)
	{
		Ui_.MenuBar_->hide ();
		Ui_.MainToolbar_->insertAction (Ui_.ActionSettings_, Ui_.ActionMenu_);
	}
	else
	{
		Ui_.MainToolbar_->removeAction (Ui_.ActionMenu_);
		Ui_.MenuBar_->show ();
	}
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

void LeechCraft::MainWindow::handleDownloadFinished (QString string)
{
	bool show = XmlSettingsManager::Instance ()->
		property ("ShowFinishedDownloadMessages").toBool ();

	HookProxy_ptr proxy (new HookProxy);
	Q_FOREACH (HookSignature<HIDDownloadFinishedNotification>::Signature_t f,
			Core::Instance ().GetHooks<HIDDownloadFinishedNotification> ())
		f (proxy, &string, show);

	if (show &&
			!proxy->IsCancelled ())
		FancyPopupManager_->ShowMessage (string);
}

void LeechCraft::MainWindow::updateIconSet ()
{
	SkinEngine::Instance ().UpdateIconSet (findChildren<QAction*> ());
	SkinEngine::Instance ().UpdateIconSet (findChildren<QTabWidget*> ());
}

void LeechCraft::MainWindow::doDelayedInit ()
{
	PluginManagerDialog *pm = new PluginManagerDialog ();
	XmlSettingsDialog_->SetCustomWidget ("PluginManager", pm);

	QObjectList settable = Core::Instance ().GetSettables ();
	for (QObjectList::const_iterator i = settable.begin (),
			end = settable.end (); i != end; ++i)
		SettingsSink_->AddDialog (*i);

	QObjectList shortcuts = Core::Instance ().GetShortcuts ();
	for (QObjectList::const_iterator i = shortcuts.begin (),
			end = shortcuts.end (); i != end; ++i)
		ShortcutManager_->AddObject (*i);

	handleShowMenuBarAsButton ();

	FillTray ();
	FillToolMenu ();

	setAcceptDrops (true);

	new StartupWizard (this);
}

void LeechCraft::MainWindow::FillTray ()
{
	QMenu *iconMenu = new QMenu (this);
	iconMenu->addAction (Ui_.ActionAddTask_);
	iconMenu->addSeparator ();
	QMenu *menu = iconMenu->addMenu (tr ("LeechCraft menu"));
	menu->addMenu (Ui_.MenuGeneral_);
	menu->addMenu (Ui_.MenuView_);
	menu->addMenu (Ui_.MenuTools_);
	menu->addMenu (Ui_.MenuHelp_);
	iconMenu->addSeparator ();

	QObjectList trayMenus = Core::Instance ()
		.GetPluginManager ()->GetAllCastableRoots<ITrayMenu*> ();
	for (QObjectList::const_iterator i = trayMenus.begin (),
			end = trayMenus.end (); i != end; ++i)
	{
		ITrayMenu* o = qobject_cast<ITrayMenu*> (*i);
		QList<QAction*> actions = o->GetTrayActions ();
		QList<QMenu*> menus = o->GetTrayMenus ();
		iconMenu->addActions (actions);
		Q_FOREACH (QMenu *m, menus)
			iconMenu->addMenu (m);

		if (actions.size () || menus.size ())
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
	QList<QMenu*> toolMenus;
	QList<QAction*> toolActions;
	Core::Instance ().GetPluginManager ()->GetAllCastableRoots<IMenuEmbedder*> ();
	Q_FOREACH (QObject *tool,
			Core::Instance ().GetPluginManager ()->
				GetAllCastableRoots<IMenuEmbedder*> ())
	{
		IMenuEmbedder *e = qobject_cast<IMenuEmbedder*> (tool);
		toolMenus += e->GetToolMenus ();
		toolActions += e->GetToolActions ();
	}

	if (toolActions.size ())
	{
		Q_FOREACH (QAction *action, toolActions)
			Ui_.MenuTools_->insertAction (Ui_.ActionLogger_, action);
		Ui_.MenuTools_->insertSeparator (Ui_.ActionLogger_);
	}

	if (toolMenus.size ())
	{
		Q_FOREACH (QMenu *menu, toolMenus)
			Ui_.MenuTools_->insertMenu (Ui_.ActionLogger_, menu);
		Ui_.MenuTools_->insertSeparator (Ui_.ActionLogger_);
	}

	on_MainTabWidget__currentChanged (0);
}

