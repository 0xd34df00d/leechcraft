#include <iostream>
#include <algorithm>
#include <QtGui/QtGui>
#include <QModelIndex>
#include <QChildEvent>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "plugininterface/proxy.h"
#include "plugininterface/graphwidget.h"
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

using namespace LeechCraft;
using namespace LeechCraft::Util;

MainWindow::MainWindow (QWidget *parent, Qt::WFlags flags)
: QMainWindow (parent, flags)
, IsShown_ (true)
, WasMaximized_ (false)
{
	QSplashScreen splash (QPixmap (":/resources/images/splashscreen.png"),
			Qt::WindowStaysOnTopHint);
	splash.show ();
	splash.showMessage (tr ("Initializing interface..."));

	if (QApplication::arguments ().contains ("-zombie"))
		QApplication::setStyle (new ZombiTechStyle ());

	installEventFilter (new ChildActionEventFilter (this));

	Ui_.setupUi (this);

	Ui_.AddTaskButton_->setDefaultAction (Ui_.ActionAddTask_);

	Ui_.ActionAddTask_->setProperty ("ActionIcon", "addjob");
	Ui_.ActionSettings_->setProperty ("ActionIcon", "settings");

	connect (Ui_.ActionAboutQt_,
			SIGNAL (triggered ()),
			qApp,
			SLOT (aboutQt ()));
	connect (Ui_.ActionMultiwindow_,
			SIGNAL (triggered ()),
			&Core::Instance (),
			SLOT (toggleMultiwindow ()));
	
	connect (Ui_.FilterCaseSensitivity_,
			SIGNAL (stateChanged (int)),
			this,
			SLOT (filterParametersChanged ()));
	connect (Ui_.FilterLine_,
			SIGNAL (textEdited (const QString&)),
			this,
			SLOT (filterParametersChanged ()));
	connect (Ui_.FilterType_,
			SIGNAL (currentIndexChanged (int)),
			this,
			SLOT (filterParametersChanged ()));

	connect (Ui_.HistoryFilterCaseSensitivity_,
			SIGNAL (stateChanged (int)),
			this,
			SLOT (historyFilterParametersChanged ()));
	connect (Ui_.HistoryFilterLine_,
			SIGNAL (textEdited (const QString&)),
			this,
			SLOT (historyFilterParametersChanged ()));
	connect (Ui_.HistoryFilterType_,
			SIGNAL (currentIndexChanged (int)),
			this,
			SLOT (historyFilterParametersChanged ()));

	SetTrayIcon ();

	FancyPopupManager_ = new FancyPopupManager (TrayIcon_, this);

	XmlSettingsDialog_ = new XmlSettingsDialog (this);
	XmlSettingsDialog_->RegisterObject (XmlSettingsManager::Instance (),
			":/coresettings.xml");
	XmlSettingsManager::Instance ()->RegisterObject ("IconSet",
			this, "updateIconSet");

	SetStatusBar ();

	ReadSettings ();

	PluginManagerDialog_ = new PluginManagerDialog (this);

	LogToolBox_ = new LogToolBox (this);

	splash.showMessage (tr ("Initializing core and plugins..."));
	connect (&Core::Instance (),
			SIGNAL (downloadFinished (const QString&)),
			this,
			SLOT (handleDownloadFinished (const QString&)));
	connect (&Core::Instance (),
			SIGNAL (log (const QString&)),
			LogToolBox_,
			SLOT (log (const QString&)));

	Core::Instance ().SetReallyMainWindow (this);
	Core::Instance ().DelayedInit ();

	QAbstractItemModel *tasksModel = Core::Instance ().GetTasksModel ();
	Ui_.PluginsTasksTree_->setModel (tasksModel);

	Ui_.HistoryView_->setModel (Core::Instance ().GetHistoryModel ());
	connect (Ui_.HistoryView_,
			SIGNAL (activated (const QModelIndex&)),
			this,
			SLOT (historyActivated (const QModelIndex&)));

	connect (Ui_.PluginsTasksTree_->selectionModel (),
			SIGNAL (currentRowChanged (const QModelIndex&,
					const QModelIndex&)),
			this,
			SLOT (updatePanes (const QModelIndex&,
					const QModelIndex&)));

	QHeaderView *itemsHeader = Ui_.PluginsTasksTree_->header ();
	QFontMetrics fm = fontMetrics ();
	itemsHeader->resizeSection (0,
			fm.width ("Average download job or torrent name is just like this one maybe."));
	itemsHeader->resizeSection (1,
			fm.width ("State of the download."));
	itemsHeader->resizeSection (2,
			fm.width ("99.99% (1234.56 kb from 2345.67 kb)"));
	itemsHeader->resizeSection (3,
			fm.width (" 1234.56 kb/s "));

	itemsHeader = Ui_.HistoryView_->header ();
	itemsHeader->resizeSection (0,
			fm.width ("Average filename or torrent name is about this width or something."));
	itemsHeader->resizeSection (1,
			fm.width ("this is some path for downloaded file with extension"));
	itemsHeader->resizeSection (2,
			fm.width (" 1234.56 kb "));
	itemsHeader->resizeSection (3,
			fm.width (QDateTime::currentDateTime ().toString ()));

	QTimer *speedUpd = new QTimer (this);
	speedUpd->setInterval (1000);
	connect (speedUpd, SIGNAL (timeout ()), this, SLOT (updateSpeedIndicators ()));
	speedUpd->start ();
	qApp->setQuitOnLastWindowClosed (false);

	updateIconSet ();

	splash.finish (this);
	show ();
}

MainWindow::~MainWindow ()
{
}

QModelIndexList MainWindow::GetSelectedRows () const
{
	return Ui_.PluginsTasksTree_->selectionModel ()->selectedRows ();
}

QTabWidget* MainWindow::GetTabWidget () const
{
	return Ui_.MainTabWidget_;
}

void MainWindow::catchError (QString message)
{
	QMessageBox::critical (this, tr ("Error"), message);
}

void MainWindow::closeEvent (QCloseEvent *e)
{
	e->ignore ();
	hide ();
	IsShown_ = false;
}

void MainWindow::SetStatusBar ()
{
	DownloadSpeed_ = new QLabel;
	DownloadSpeed_->setText ("0");
	DownloadSpeed_->setMinimumWidth (70);
	DownloadSpeed_->setAlignment (Qt::AlignRight);
	UploadSpeed_ = new QLabel;
	UploadSpeed_->setText ("0");
	UploadSpeed_->setMinimumWidth (70);
	UploadSpeed_->setAlignment (Qt::AlignRight);

	DSpeedGraph_ = new GraphWidget (Qt::green);
	DSpeedGraph_->setMinimumWidth (250);
	USpeedGraph_ = new GraphWidget (Qt::yellow);
	USpeedGraph_->setMinimumWidth (250);

	statusBar ()->addPermanentWidget (DownloadSpeed_);
	statusBar ()->addPermanentWidget (UploadSpeed_);
	statusBar ()->addPermanentWidget (DSpeedGraph_);
	statusBar ()->addPermanentWidget (USpeedGraph_);
}

void MainWindow::SetTrayIcon ()
{
	TrayIcon_ = new QSystemTrayIcon (QIcon (":/resources/images/mainapp.png"), this);

	QMenu *iconMenu = new QMenu (this);
	iconMenu->addAction (tr ("Show/hide"), this, SLOT (showHideMain ()));
	iconMenu->addSeparator ();
	iconMenu->addAction (Ui_.ActionAddTask_);
	iconMenu->addSeparator ();
	iconMenu->addAction (tr ("Quit"),
			this,
			SLOT (on_ActionQuit__triggered ()));

	TrayIcon_->setContextMenu (iconMenu);
	TrayIcon_->show ();
	connect (TrayIcon_,
			SIGNAL (activated (QSystemTrayIcon::ActivationReason)),
			this,
			SLOT (handleTrayIconActivated (QSystemTrayIcon::ActivationReason)));
}

void MainWindow::ReadSettings ()
{
	QSettings settings ("Deviant", "Leechcraft");
	settings.beginGroup ("geometry");
	resize (settings.value ("size", QSize  (750, 550)).toSize ());
	move   (settings.value ("pos",  QPoint (10, 10)).toPoint ());
	WasMaximized_ = settings.value ("maximized").toBool ();
	WasMaximized_ ? showMaximized () : showNormal ();
	settings.endGroup ();
}

void MainWindow::WriteSettings ()
{
	QSettings settings ("Deviant", "Leechcraft");
	settings.beginGroup ("geometry");
	settings.setValue ("size", size ());
	settings.setValue ("pos",  pos ());
	settings.setValue ("maximized", isMaximized ());
	settings.endGroup ();
}

void MainWindow::on_ActionAboutLeechCraft__triggered ()
{
	QMessageBox::information (this, tr ("Information"),
			tr ("<img src=\":/resources/images/mainapp.png\" /><h1>LeechCraft 0.3.0_pre</h1>"
				"LeechCraft is a cross-platform extensible download manager. Currently it offers "
				"full-featured BitTorrent client, feed reader, HTTP support, Remote access "
				"and much more. It also aims to be resource-efficient working quite well on "
				"even old computers.<br /><br />Here are some useful links for you:<br />"
				"<a href=\"http://bugs.deviant-soft.ws\">Bugtracker and feature request tracker</a><br />"
				"<a href=\"http://sourceforge.net/project/showfiles.php?group_id=161819\">Latest file releases</a><br />"
				"<a href=\"http://deviant-soft.ws\">LeechCraft's Site</a><br />"
				"<a href=\"http://sourceforge.net/projects/leechcraft\">LeechCraft's site at sourceforge.net</a><br />"));
}

void MainWindow::on_ActionAddTask__triggered ()
{
	CommonJobAdder adder (this);
	if (adder.exec () != QDialog::Accepted)
		return;

	QString name = adder.GetString ();
	if (!name.isEmpty ())
		Core::Instance ().TryToAddJob (name, adder.GetWhere ());
}

void MainWindow::on_ActionSettings__triggered ()
{
	XmlSettingsDialog_->show ();
	XmlSettingsDialog_->setWindowTitle (windowTitle () + tr (": Preferences"));
}

void MainWindow::on_ActionQuit__triggered ()
{
	WriteSettings ();
	Core::Instance ().Release ();

	TrayIcon_->hide ();
	delete TrayIcon_;
	qDebug () << "Releasing XmlSettingsManager";
	delete XmlSettingsDialog_;
	XmlSettingsManager::Instance ()->Release ();
	qDebug () << "Destroyed fine";

	qApp->quit ();
}

void MainWindow::on_ActionFullscreenMode__triggered (bool full)
{
	qDebug () << Q_FUNC_INFO << full << WasMaximized_;
	if (full)
	{
		WasMaximized_ = isMaximized ();
		showFullScreen ();
	}
	else if (WasMaximized_)
	{
		showMaximized ();
		// Because shit happens on X11 otherwise
		QTimer::singleShot (200,
				this,
				SLOT (showMaximized ()));
	}
	else
		showNormal ();
}

void MainWindow::on_ActionLogger__triggered ()
{
	LogToolBox_->show ();
}

void MainWindow::updatePanes (const QModelIndex& newIndex,
		const QModelIndex& oldIndex)
{
	Core::Instance ().SetNewRow (newIndex);

	if (Core::Instance ().SameModel (newIndex, oldIndex))
		return;

	if (Ui_.PluginsStuff_->count () >= 3)
	{
		if (Ui_.PluginsStuff_->count () == 4)
			Ui_.PluginsStuff_->takeAt (3)->widget ()->hide ();
		Ui_.PluginsStuff_->takeAt (1)->widget ()->hide ();
	}

	if (newIndex.isValid ())
	{
		QWidget *controls = Core::Instance ()
					.GetControls (newIndex),
				*addiInfo = Core::Instance ()
					.GetAdditionalInfo (newIndex);
		Ui_.PluginsStuff_->insertWidget (1, controls);
		controls->show ();
		if (addiInfo)
		{
			Ui_.PluginsStuff_->addWidget (addiInfo);
			addiInfo->show ();
		}
	}
}

void MainWindow::updateSpeedIndicators ()
{
	QPair<qint64, qint64> speeds = Core::Instance ().GetSpeeds ();

	DownloadSpeed_->setText (Proxy::Instance ()->MakePrettySize (speeds.first) + tr ("/s"));
	UploadSpeed_->setText (Proxy::Instance ()->MakePrettySize (speeds.second) + tr ("/s"));
	DSpeedGraph_->PushSpeed (speeds.first);
	USpeedGraph_->PushSpeed (speeds.second);
}

void MainWindow::showHideMain ()
{
	IsShown_ = 1 - IsShown_;
	IsShown_ ? show () : hide ();
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

void MainWindow::handleDownloadFinished (const QString& string)
{
	if (XmlSettingsManager::Instance ()->property ("ShowFinishedDownloadMessages").toBool ())
		FancyPopupManager_->ShowMessage (string);
}

void MainWindow::filterParametersChanged ()
{
	Core::FilterType ft;
	switch (Ui_.FilterType_->currentIndex ())
	{
		case 0:
			ft = Core::FTFixedString;
			break;
		case 1:
			ft = Core::FTWildcard;
			break;
		case 2:
			ft = Core::FTRegexp;
			break;
		case 3:
			ft = Core::FTTags;
			break;
		default:
			qWarning () << Q_FUNC_INFO
				<< "unhandled ft"
				<< Ui_.FilterType_->currentIndex ();
			return;
	}

	bool caseSensitivity = (Ui_.FilterCaseSensitivity_->checkState () == Qt::Checked);
	Core::Instance ().UpdateFiltering (Ui_.FilterLine_->text (),
			ft, caseSensitivity);
}

void MainWindow::historyFilterParametersChanged ()
{
	Core::FilterType ft;
	switch (Ui_.HistoryFilterType_->currentIndex ())
	{
		case 0:
			ft = Core::FTFixedString;
			break;
		case 1:
			ft = Core::FTWildcard;
			break;
		case 2:
			ft = Core::FTRegexp;
			break;
		case 3:
			ft = Core::FTTags;
			break;
		default:
			qWarning () << Q_FUNC_INFO
				<< "unhandled ft"
				<< Ui_.HistoryFilterType_->currentIndex ();
			return;
	}

	bool caseSensitivity =
		(Ui_.HistoryFilterCaseSensitivity_->checkState () == Qt::Checked);
	Core::Instance ().UpdateFiltering (Ui_.HistoryFilterLine_->text (),
			ft, caseSensitivity, true);
}

void MainWindow::updateIconSet ()
{
	SkinEngine::Instance ().updateIconSet (findChildren<QAction*> ());
}

void MainWindow::on_ActionPluginManager__triggered ()
{
	PluginManagerDialog_->show ();
}

void MainWindow::historyActivated (const QModelIndex& index)
{
	Core::Instance ().HistoryActivated (index.row ());
}

