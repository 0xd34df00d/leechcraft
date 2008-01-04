#include <QtGui>
#include <QMutex>
#include <iostream>
#include "plugininterface/proxy.h"
#include "exceptions/notimplemented.h"
#include "mainwindow.h"
#include "view.h"
#include "core.h"
#include "logshower.h"
#include "plugininfo.h"
#include "pluginlisttablewidgeticon.h"
#include "changelogdialog.h"

MainWindow *MainWindow::Instance_ = 0;
QMutex *MainWindow::InstanceMutex_ = new QMutex;

MainWindow::MainWindow (QWidget *parent, Qt::WFlags flags)
: QMainWindow (parent, flags)
, SettingsClearScheduled_ (false)
, IsShown_ (true)
{
	QSplashScreen splash (QPixmap (":/resources/images/splashscreen.png"), Qt::WindowStaysOnTopHint);
	splash.show ();

	splash.showMessage (tr ("Initializing interface..."));
	statusBar ();
	SetTrayIcon ();
	SetupToolbars ();
	SetupActions ();
	SetupMenus ();
	setWindowIcon (QIcon (":/resources/images/mainapp.png"));
    setWindowTitle (QCoreApplication::applicationName ());

	DownloadSpeed_ = new QLabel;
	DownloadSpeed_->setText ("0");
	DownloadSpeed_->setMinimumWidth (70);
	DownloadSpeed_->setAlignment (Qt::AlignRight);
	UploadSpeed_ = new QLabel;
	UploadSpeed_->setText ("0");
	UploadSpeed_->setMinimumWidth (70);
	UploadSpeed_->setAlignment (Qt::AlignRight);

	statusBar ()->addPermanentWidget (DownloadSpeed_);
	statusBar ()->addPermanentWidget (UploadSpeed_);

	splash.showMessage (tr ("Reading settings..."));
	ReadSettings ();

	splash.showMessage (tr ("Initializing core and plugins..."));
	Model_ = new Core (this);
	connect (Model_, SIGNAL (gotPlugin (const PluginInfo*)), this, SLOT (addPluginToList (const PluginInfo*)));
	Model_->SetReallyMainWindow (this);
	Model_->DoDelayedInit ();

	QTimer *speedUpd = new QTimer (this);
	speedUpd->setInterval (1000);
	connect (speedUpd, SIGNAL (timeout ()), this, SLOT (updateSpeedIndicators ()));
	speedUpd->start ();

	show ();
	
	splash.finish (this);

	connect (Proxy::Instance (), SIGNAL (addMessage (const QString&, bool)), this, SLOT (handleAddMessage (const QString&, bool)));
}

MainWindow::~MainWindow ()
{
	if (SettingsClearScheduled_)
	{
		QSettings settings ("Deviant", "Leechcraft");
		settings.clear ();
	}
}

MainWindow* MainWindow::Instance ()
{
	if (!Instance_)
		Instance_ = new MainWindow ();
	return Instance_;
}

void MainWindow::catchError (QString message, Errors::Severity error)
{
	switch (error)
	{
		case Errors::Notice:
			QMessageBox::information (this, tr ("Notice"), message);
			break;
		case Errors::Warning:
			QMessageBox::warning (this, tr ("Warning"), message);
			break;
		case Errors::Error:
			QMessageBox::critical (this, tr ("Error"), message);
			break;
		case Errors::Critical:
			QMessageBox::critical (this, tr ("Critical"), message);
			qApp->quit ();
			break;
	}
}

void MainWindow::closeEvent (QCloseEvent *e)
{
	if (QMessageBox::question (this, tr ("Question"), tr ("Do you really want to exit?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
	{
		e->ignore ();
		return;
	}

	TrayIcon_->hide ();
    WriteSettings ();
	delete Model_;
	qApp->quit ();
    e->accept ();
}

void MainWindow::SetupToolbars ()
{
	ToolToolbar_ = addToolBar (tr ("Tools"));
}

void MainWindow::SetupActions ()
{
	BackupSettings_ = ToolToolbar_->addAction (tr ("Backup settings..."), this, SLOT (backupSettings ()));
	RestoreSettings_ = ToolToolbar_->addAction (tr ("Restore settings..."), this, SLOT (restoreSettings ()));
}

void MainWindow::SetupMenus ()
{
    File_		= menuBar ()->addMenu (tr ("&File"));
	ToolsMenu_	= menuBar ()->addMenu (tr ("&Tools"));
    Help_		= menuBar ()->addMenu (tr ("&Help"));

    FillMenus ();
}

void MainWindow::SetTrayIcon ()
{
	QMenu *iconMenu = new QMenu;
	iconMenu->addAction (tr ("Show/hide main"), this, SLOT (showHideMain ()));
	iconMenu->addAction (tr ("Hide all"), this, SLOT (hideAll ()));
	iconMenu->addSeparator ();
	iconMenu->addAction (tr ("Exit"), qApp, SLOT (quit ()));

	TrayIcon_ = new QSystemTrayIcon (QIcon (":/resources/images/mainapp.png"), this);
	TrayIcon_->setContextMenu (iconMenu);
	TrayIcon_->show ();
	connect (TrayIcon_, SIGNAL (activated (QSystemTrayIcon::ActivationReason)), this, SLOT (handleTrayIconActivated (QSystemTrayIcon::ActivationReason)));
}

void MainWindow::FillMenus ()
{
    MakeActions ();
}

void MainWindow::MakeActions ()
{
	QAction *a = File_->addAction (tr ("&Quit"), qApp, SLOT (quit ()));
	a->setStatusTip (tr ("Exit from application"));

	ToolsMenu_->addAction (BackupSettings_);
	ToolsMenu_->addAction (RestoreSettings_);
	Help_->addAction (tr ("&Changelog..."), this, SLOT (showChangelog ()));
	Help_->addAction (tr ("&About Qt..."), qApp, SLOT (aboutQt ()));
}

void MainWindow::ReadSettings ()
{
	QSettings settings ("Deviant", "Leechcraft");
    settings.beginGroup ("geometry");
    resize (settings.value ("size", QSize  (800, 550)).toSize ());
    move   (settings.value ("pos",  QPoint (10, 10)).toPoint ());
    settings.value ("maximized").toBool () ? showMaximized () : showNormal ();
	InitializeMainView (settings.value ("pluginListHorizontalHeaderState").toByteArray ());
    settings.endGroup ();
}

void MainWindow::WriteSettings ()
{
	QSettings settings ("Deviant", "Leechcraft");
    settings.beginGroup ("geometry");
    settings.setValue ("size", size ());
    settings.setValue ("pos",  pos ());
    settings.setValue ("maximized", isMaximized ());
	settings.setValue ("pluginListHorizontalHeaderState", PluginsList_->header ()->saveState ());
    settings.endGroup ();
}

void MainWindow::InitializeMainView (const QByteArray& pluginliststate)
{
	Splitter_ = new QSplitter (Qt::Vertical, this);
	setCentralWidget (Splitter_);
	
	PluginsList_ = new QTreeWidget (this);
	PluginsList_->header ()->setClickable (false);
	PluginsList_->setUniformRowHeights (true);
	PluginsList_->setSelectionBehavior (QAbstractItemView::SelectRows);
	PluginsList_->setSelectionMode (QAbstractItemView::SingleSelection);
	PluginsList_->setEditTriggers (QAbstractItemView::NoEditTriggers);
	PluginsList_->setItemsExpandable (true);
	QStringList headerLabels;
	headerLabels << tr ("Plugin") << tr ("Plugin details");
	PluginsList_->setHeaderLabels (headerLabels);
	PluginsList_->header ()->setHighlightSections (false);
	PluginsList_->header ()->setDefaultAlignment (Qt::AlignLeft);
	PluginsList_->header ()->setResizeMode (QHeaderView::ResizeToContents);
	
	connect (PluginsList_, SIGNAL (itemDoubleClicked (QTreeWidgetItem*, int)), this, SLOT (handlePluginsListDoubleClick (QTreeWidgetItem*, int)));
		
	if (!pluginliststate.isNull () && !pluginliststate.isEmpty ())
	{
		PluginsList_->header ()->restoreState (pluginliststate);
	}

	PluginsList_->header ()->setStretchLastSection (true);
	
	LogShower_ = new LogShower;

	Splitter_->addWidget (PluginsList_);
	Splitter_->addWidget (LogShower_);
	Splitter_->show ();
}

void MainWindow::handlePluginsListDoubleClick (QTreeWidgetItem *item, int column)
{
	int index = PluginsList_->indexOfTopLevelItem (item);

	if (index != -1)
	{
		if (item->isDisabled ())
			return;

		if (Model_->ShowPlugin (PluginsList_->indexOfTopLevelItem (item)))
			if (column == 0)
				PluginsList_->isItemExpanded (item) ? PluginsList_->collapseItem (item) : PluginsList_->expandItem (item);
	}
}

void MainWindow::addPluginToList (const PluginInfo *pInfo)
{
	QTreeWidgetItem *item = new QTreeWidgetItem ();
	item->setData (0, Qt::DecorationRole, pInfo->GetIcon ());
	item->setData (0, Qt::StatusTipRole, pInfo->GetStatusbarMessage ());
	item->setData (0, Qt::DisplayRole, pInfo->GetName ());
	item->setText (1, pInfo->GetInfo ());
	PluginsList_->addTopLevelItem (item);

	QStringList provides = pInfo->GetProvides (),
				needs = pInfo->GetNeeds (),
				uses = pInfo->GetUses ();

	QBrush failedDep (Qt::red);
	if (!pInfo->GetDependenciesMet ())
		item->setDisabled (true);

	if (provides.size ())
	{
		QTreeWidgetItem *header = new QTreeWidgetItem (item);
		header->setText (0, tr ("Provides:"));
		for (int i = 0; i < provides.size (); ++i)
		{
			QTreeWidgetItem *p = new QTreeWidgetItem (header);
			p->setFirstColumnSpanned (true);
			p->setText (0, provides.at (i));
		}
	}

	if (needs.size ())
	{
		QTreeWidgetItem *header = new QTreeWidgetItem (item);
		header->setText (0, tr ("Needs:"));
		for (int i = 0; i < needs.size (); ++i)
		{
			QTreeWidgetItem *n = new QTreeWidgetItem (header);
			n->setText (0, needs.at (i));
			n->setFirstColumnSpanned (true);
			if (!pInfo->GetDependenciesMet ())
				if (pInfo->GetFailedDeps ().contains (needs.at (i)))
					n->setForeground (0, failedDep);
		}
	}

	if (uses.size ())
	{
		QTreeWidgetItem *header = new QTreeWidgetItem (item);
		header->setText (0, tr ("Uses:"));
		for (int i = 0; i < uses.size (); ++i)
		{
			QTreeWidgetItem *u = new QTreeWidgetItem (header);
			u->setText (0, uses.at (i));
			u->setFirstColumnSpanned (true);
		}
	}

	delete pInfo;
}

void MainWindow::updateSpeedIndicators ()
{
	QPair<qint64, qint64> speeds = Model_->GetSpeeds ();

	DownloadSpeed_->setText (Proxy::Instance ()->MakePrettySize (speeds.first) + tr ("/s"));
	UploadSpeed_->setText (Proxy::Instance ()->MakePrettySize (speeds.second) + tr ("/s"));
}

void MainWindow::handleAddMessage (const QString& msg, bool uploading)
{
	if (uploading)
		LogShower_->AddUploadMessage (msg);
	else
		LogShower_->AddDownloadMessage (msg);
}

void MainWindow::backupSettings ()
{
	QSettings settings ("Deviant", "Leechcraft");
	QString filename = QFileDialog::getSaveFileName (this, tr ("Backup file"), QString (), tr ("Settings files (*.ini)"));
	if (filename.isEmpty ())
		return;

	if (filename.right (4).toLower () != QString (".ini"))
		filename += ".ini";

	QSettings backupSettings (filename, QSettings::IniFormat);
	QStringList allKeys = settings.allKeys ();

	for (int i = 0; i < allKeys.size (); ++i)
		backupSettings.setValue (allKeys [i], settings.value (allKeys [i]));

	QMessageBox::information (this, tr ("Finished"), tr ("Settings sucessfully backuped to %1").arg (backupSettings.fileName ()));
}

void MainWindow::restoreSettings ()
{
	QSettings settings ("Deviant", "Leechcraft");
	QString filename = QFileDialog::getOpenFileName (this, tr ("Backup file"), QString (), tr ("Settings files (*.ini)"));
	if (filename.isEmpty () || !QFile::exists (filename))
		return;

	QSettings backupSettings (filename, QSettings::IniFormat);
	settings.clear ();
	QStringList allKeys = backupSettings.allKeys ();
	for (int i = 0; i < allKeys.size (); ++i)
		settings.setValue (allKeys [i], backupSettings.value (allKeys [i]));

	QMessageBox::information (this, tr ("Finished"), tr ("Settings sucessfully restored from %1").arg (backupSettings.fileName ()));
}

void MainWindow::clearSettings (bool scheduled)
{
}

void MainWindow::showChangelog ()
{
	ChangelogDialog ce (this);
	ce.exec ();
}

void MainWindow::showHideMain ()
{
	IsShown_ = 1 - IsShown_;
	IsShown_ ? show () : hide ();
}

void MainWindow::hideAll ()
{
}

void MainWindow::handleTrayIconActivated (QSystemTrayIcon::ActivationReason reason)
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

