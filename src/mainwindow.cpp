#include <iostream>
#include <algorithm>
#include <QtGui/QtGui>
#include <QModelIndex>
#include <QChildEvent>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "plugininterface/proxy.h"
#include "mainwindow.h"
#include "view.h"
#include "core.h"
#include "commonjobadder.h"
#include "xmlsettingsmanager.h"
#include "pluginmanagerdialog.h"
#include "fancypopupmanager.h"
#include "skinengine.h"
#include "childactioneventfilter.h"
#include "zombitechstyle.h"
#include "logtoolbox.h"
#include "settingssink.h"
#include "iconchooser.h"
#include "graphwidget.h"
#include "shortcutmanager.h"
#include "appstyler.h"
#include "tagsviewer.h"
#include "application.h"

using namespace LeechCraft;
using namespace LeechCraft::Util;

LeechCraft::MainWindow::MainWindow (QWidget *parent, Qt::WFlags flags)
: QMainWindow (parent, flags)
, IsShown_ (true)
, WasMaximized_ (false)
, FilterTimer_ (new QTimer (this))
, CurrentToolBar_ (0)
{
	InitializeInterface ();

	ShowMessage (tr ("Initializing core and plugins..."));
	connect (qApp,
			SIGNAL (aboutToQuit ()),
			this,
			SLOT (handleQuit ()));

	connect (&Core::Instance (),
			SIGNAL (loadProgress (const QString&)),
			this,
			SLOT (handleLoadProgress (const QString&)));
	connect (&Core::Instance (),
			SIGNAL (downloadFinished (const QString&)),
			this,
			SLOT (handleDownloadFinished (const QString&)));
	connect (&Core::Instance (),
			SIGNAL (log (const QString&)),
			LogToolBox_,
			SLOT (log (const QString&)));

	FilterTimer_->setSingleShot (true);
	FilterTimer_->setInterval (800);
	connect (FilterTimer_,
			SIGNAL (timeout ()),
			this,
			SLOT (feedFilterParameters ()));

	Core::Instance ().SetReallyMainWindow (this);
	Core::Instance ().DelayedInit ();

	PluginManagerDialog_ = new PluginManagerDialog (this);

	ShowMessage (tr ("Initializing core and plugins..."));

	QAbstractItemModel *tasksModel = Core::Instance ().GetTasksModel ();
	Ui_.PluginsTasksTree_->setModel (tasksModel);

	connect (Ui_.PluginsTasksTree_->selectionModel (),
			SIGNAL (selectionChanged (const QItemSelection&,
					const QItemSelection&)),
			this,
			SLOT (updatePanes (const QItemSelection&,
					const QItemSelection&)));

	QHeaderView *itemsHeader = Ui_.PluginsTasksTree_->header ();
	QFontMetrics fm = fontMetrics ();
	itemsHeader->resizeSection (0,
			fm.width ("Average download job or torrent name is just like this."));
	itemsHeader->resizeSection (1,
			fm.width ("Of the download."));
	itemsHeader->resizeSection (2,
			fm.width ("99.99% (1024.0 kb from 1024.0 kb at 1024.0 kb/s)"));

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

	QObjectList settable = Core::Instance ().GetSettables ();
	for (QObjectList::const_iterator i = settable.begin (),
			end = settable.end (); i != end; ++i)
		SettingsSink_->AddDialog (*i);

	QList<QAction*> actions2embed = Core::Instance ().GetActions2Embed ();
	Q_FOREACH (QAction *act, actions2embed)
		act->setParent (this);
	if (actions2embed.size ())
		addToolBar (tr ("Plugins"))->addActions (actions2embed);

	updateIconSet ();

	QObjectList shortcuts = Core::Instance ().GetShortcuts ();
	for (QObjectList::const_iterator i = shortcuts.begin (),
			end = shortcuts.end (); i != end; ++i)
		ShortcutManager_->AddObject (*i);

	filterParametersChanged ();

	setUpdatesEnabled (true);
	SplashScreen_->finish (this);
	show ();
	delete SplashScreen_;

	WasMaximized_ = isMaximized ();
	Ui_.ActionFullscreenMode_->setChecked (isFullScreen ());
}

LeechCraft::MainWindow::~MainWindow ()
{
}

QModelIndexList LeechCraft::MainWindow::GetSelectedRows () const
{
	return Ui_.PluginsTasksTree_->selectionModel ()->selectedRows ();
}

TabWidget* LeechCraft::MainWindow::GetTabWidget () const
{
	return Ui_.MainTabWidget_;
}

const IShortcutProxy* LeechCraft::MainWindow::GetShortcutProxy () const
{
	return ShortcutManager_;
}

QTreeView* LeechCraft::MainWindow::GetMainView () const
{
	return Ui_.PluginsTasksTree_;
}

void LeechCraft::MainWindow::SetAdditionalTitle (const QString& title)
{
	if (title.isEmpty ())
		setWindowTitle (tr ("LeechCraft"));
	else
		setWindowTitle (tr ("%1 - LeechCraft").arg (title));
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
	hide ();
	IsShown_ = false;
}

void LeechCraft::MainWindow::InitializeInterface ()
{
	setUpdatesEnabled (false);
	SplashScreen_ = new QSplashScreen (QPixmap (":/resources/images/splashscreen.png"));
	SplashScreen_->show ();
	ShowMessage (tr ("Initializing interface..."));

	if (QApplication::arguments ().contains ("-zombie"))
		QApplication::setStyle (new ZombiTechStyle ());

	installEventFilter (new ChildActionEventFilter (this));

	Ui_.setupUi (this);

	Ui_.ActionAddTask_->setProperty ("ActionIcon", "addjob");
	Ui_.ActionSettings_->setProperty ("ActionIcon", "settings");
	Ui_.ActionQuit_->setProperty ("ActionIcon", "exit");
	Ui_.ActionPluginManager_->setProperty ("ActionIcon", "pluginmanager");
	Ui_.ActionLogger_->setProperty ("ActionIcon", "logger");
	Ui_.ActionFullscreenMode_->setProperty ("ActionIcon", "fullscreen");
	Ui_.ActionFullscreenMode_->setParent (this);

	Ui_.MainTabWidget_->setProperty ("TabIcons", "downloaders");
	Ui_.ControlsDockWidget_->hide ();

	connect (Ui_.FilterLine_,
			SIGNAL (textEdited (const QString&)),
			this,
			SLOT (filterParametersChanged ()));
	connect (Ui_.FilterLine_,
			SIGNAL (returnPressed ()),
			this,
			SLOT (filterReturnPressed ()));

	SetTrayIcon ();

	XmlSettingsDialog_ = new XmlSettingsDialog ();
	XmlSettingsDialog_->RegisterObject (XmlSettingsManager::Instance (),
			"coresettings.xml");

	XmlSettingsDialog_->SetCustomWidget ("TagsViewer", new TagsViewer);

	XmlSettingsManager::Instance ()->RegisterObject ("Language",
			this, "handleLanguage");

	IconChooser *ic = new IconChooser (SkinEngine::Instance ().ListIcons (),
			this);
	connect (ic,
			SIGNAL (requestNewIconSet ()),
			this,
			SLOT (updateIconSet ()));
	XmlSettingsDialog_->SetCustomWidget ("IconSet", ic);
	XmlSettingsDialog_->SetCustomWidget ("AppQStyle", new AppStyler (this));

	SettingsSink_ = new SettingsSink (tr ("LeechCraft"),
			XmlSettingsDialog_);
	ShortcutManager_ = new ShortcutManager (this);
	XmlSettingsDialog_->SetCustomWidget ("ShortcutManager", ShortcutManager_);

	SetStatusBar ();
	ReadSettings ();

	FancyPopupManager_ = new FancyPopupManager (TrayIcon_, this);
	LogToolBox_ = new LogToolBox (this);

	setAcceptDrops (true);
}


void LeechCraft::MainWindow::SetStatusBar ()
{
	QFontMetrics fm = fontMetrics ();
	int minSize = fm.width (Proxy::Instance ()->MakePrettySize (999) + tr ("/s	"));

	DownloadSpeed_ = new QLabel;
	DownloadSpeed_->setText (Proxy::Instance ()->MakePrettySize (0) + tr ("/s"));
	DownloadSpeed_->setMinimumWidth (minSize);
	DownloadSpeed_->setAlignment (Qt::AlignRight);
	UploadSpeed_ = new QLabel;
	UploadSpeed_->setText (Proxy::Instance ()->MakePrettySize (0) + tr ("/s"));
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

void LeechCraft::MainWindow::SetTrayIcon ()
{
	TrayIcon_ = new QSystemTrayIcon (QIcon (":/resources/images/leechcraft.png"), this);

	QMenu *iconMenu = new QMenu (this);
	iconMenu->addAction (Ui_.ActionAddTask_);
	iconMenu->addSeparator ();
	iconMenu->addAction (Ui_.ActionQuit_);

	TrayIcon_->setContextMenu (iconMenu);
	TrayIcon_->show ();
	connect (TrayIcon_,
			SIGNAL (activated (QSystemTrayIcon::ActivationReason)),
			this,
			SLOT (handleTrayIconActivated (QSystemTrayIcon::ActivationReason)));
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
}

void LeechCraft::MainWindow::WriteSettings ()
{
	QSettings settings ("Deviant", "Leechcraft");
	settings.beginGroup ("geometry");
	settings.setValue ("size", size ());
	settings.setValue ("pos",  pos ());
	settings.setValue ("maximized", isMaximized ());
	settings.endGroup ();
}

void LeechCraft::MainWindow::ShowMessage (const QString& message)
{
	SplashScreen_->showMessage (message,
			Qt::AlignLeft | Qt::AlignBottom,
			QApplication::palette ().color (QPalette::WindowText));
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

void LeechCraft::MainWindow::on_ActionSettings__triggered ()
{
	SettingsSink_->show ();
}

void LeechCraft::MainWindow::on_ActionQuit__triggered ()
{
	if (XmlSettingsManager::Instance ()->property ("ConfirmQuit").toBool () &&
			QMessageBox::question (this,
				tr ("LeechCraft"),
				tr ("Do you really want to quit?"),
				QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
		return;
	qApp->quit ();
}

void LeechCraft::MainWindow::handleQuit ()
{
	setUpdatesEnabled (false);
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
	if (QMessageBox::question (this,
				tr ("LeechCraft"),
				tr ("Changing language requires restarting LeechCraft. "
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
	if (CurrentToolBar_)
	{
		removeToolBar (CurrentToolBar_);
		CurrentToolBar_ = 0;
	}

	if (index == 0)
	{
		QItemSelection sel = Ui_.PluginsTasksTree_->
			selectionModel ()->selection ();
		if (sel.size ())
		{
			QModelIndex ri = sel.at (0).topLeft ();
			QToolBar *controls = Core::Instance ()
						.GetControls (ri);
			QWidget *addiInfo = Core::Instance ()
						.GetAdditionalInfo (ri);

			if (controls)
			{
				controls->setWindowTitle (tr ("Plugin control"));
				controls->setFloatable (true);
				controls->setMovable (true);
				addToolBar (controls);
				controls->show ();
			}
			if (addiInfo)
			{
				if (addiInfo->parent () != this)
					addiInfo->setParent (this);
				Ui_.ControlsDockWidget_->setWidget (addiInfo);
				Ui_.ControlsDockWidget_->show ();
			}
		}
	}
	else
	{
		QItemSelection sel = Ui_.PluginsTasksTree_->
			selectionModel ()->selection ();
		if (sel.size ())
		{
			QModelIndex ri = sel.at (0).topLeft ();
			QToolBar *controls = Core::Instance ()
						.GetControls (ri);

			if (controls)
				removeToolBar (controls);
		}

		CurrentToolBar_ = Core::Instance ().GetToolBar (index);
		if (CurrentToolBar_)
		{
			addToolBar (CurrentToolBar_);
			CurrentToolBar_->show ();
		}
	}
}

void LeechCraft::MainWindow::updatePanes (const QItemSelection& newIndexes,
		const QItemSelection& oldIndexes)
{
#ifdef QT_DEBUG
	qDebug () << Q_FUNC_INFO;
#endif

	QModelIndex oldIndex, newIndex;
	if (oldIndexes.size ())
		oldIndex = oldIndexes.at (0).topLeft ();
	if (newIndexes.size ())
		newIndex = newIndexes.at (0).topLeft ();

	if (!newIndex.isValid ())
	{
#ifdef QT_DEBUG
		qDebug () << "invalidating";
#endif

		Core::Instance ().SetNewRow (newIndex);
		if (oldIndex.isValid ())
		{
#ifdef QT_DEBUG
			qDebug () << "erasing older stuff";
#endif
			QToolBar *oldControls = Core::Instance ().GetControls (oldIndex);
			if (oldControls)
				removeToolBar (oldControls);
			Ui_.ControlsDockWidget_->hide ();
		}
	}
	else if (oldIndex.isValid () &&
			Core::Instance ().SameModel (newIndex, oldIndex))
	{
#ifdef QT_DEBUG
		qDebug () << "setting new row";
#endif
		Core::Instance ().SetNewRow (newIndex);
	}
	else if (newIndex.isValid ())
	{
		QToolBar *controls = Core::Instance ()
					.GetControls (newIndex);
		QWidget *addiInfo = Core::Instance ()
					.GetAdditionalInfo (newIndex);

		if (oldIndex.isValid ())
		{
#ifdef QT_DEBUG
			qDebug () << "erasing older stuff";
#endif
			QToolBar *oldControls = Core::Instance ().GetControls (oldIndex);
			if (oldControls)
				removeToolBar (oldControls);
			Ui_.ControlsDockWidget_->hide ();
		}

#ifdef QT_DEBUG
		qDebug () << "inserting newer stuff" << newIndex << controls << addiInfo;
#endif

		Core::Instance ().SetNewRow (newIndex);
		
		if (controls)
		{
			controls->setWindowTitle (tr ("Plugin control"));
			controls->setFloatable (true);
			controls->setMovable (true);
			addToolBar (controls);
			controls->show ();
		}
		if (addiInfo)
		{
			if (addiInfo->parent () != this)
				addiInfo->setParent (this);
			Ui_.ControlsDockWidget_->setWidget (addiInfo);
			Ui_.ControlsDockWidget_->show ();
		}
	}
}

void LeechCraft::MainWindow::updateSpeedIndicators ()
{
	QPair<qint64, qint64> speeds = Core::Instance ().GetSpeeds ();

	QString down = Proxy::Instance ()->MakePrettySize (speeds.first) + tr ("/s");
	QString up = Proxy::Instance ()->MakePrettySize (speeds.second) + tr ("/s");
	DownloadSpeed_->setText (down);
	UploadSpeed_->setText (up);
	SpeedGraph_->PushSpeed (speeds.first, speeds.second);

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

void LeechCraft::MainWindow::handleDownloadFinished (const QString& string)
{
	bool show = XmlSettingsManager::Instance ()->
		property ("ShowFinishedDownloadMessages").toBool ();

	HookProxy_ptr proxy (new HookProxy);
	Q_FOREACH (HookSignature<HIDDownloadFinishedNotification>::Signature_t f,
			Core::Instance ().GetHooks<HIDDownloadFinishedNotification> ())
		f (proxy, string, show);

	if (show &&
			!proxy->IsCancelled ())
		FancyPopupManager_->ShowMessage (string);
}

void LeechCraft::MainWindow::filterParametersChanged ()
{
	FilterTimer_->stop ();
	FilterTimer_->start ();
}

void LeechCraft::MainWindow::filterReturnPressed ()
{
	FilterTimer_->stop ();
	feedFilterParameters ();
}

void LeechCraft::MainWindow::feedFilterParameters ()
{
	Core::Instance ().UpdateFiltering (Ui_.FilterLine_->text ());
}

void LeechCraft::MainWindow::updateIconSet ()
{
	SkinEngine::Instance ().UpdateIconSet (findChildren<QAction*> ());
	SkinEngine::Instance ().UpdateIconSet (findChildren<QTabWidget*> ());
}

void LeechCraft::MainWindow::on_ActionPluginManager__triggered ()
{
	PluginManagerDialog_->show ();
}

void LeechCraft::MainWindow::handleLoadProgress (const QString& msg)
{
	ShowMessage (tr ("Initializing core and plugins...") + " " + msg);
}

