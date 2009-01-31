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
#include "settingssink.h"

using namespace LeechCraft;
using namespace LeechCraft::Util;

MainWindow::MainWindow (QWidget *parent, Qt::WFlags flags)
: QMainWindow (parent, flags)
, IsShown_ (true)
, WasMaximized_ (false)
{
	setUpdatesEnabled (false);
	SplashScreen_ = new QSplashScreen (QPixmap (":/resources/images/splashscreen.png"));
	SplashScreen_->show ();
	SplashScreen_->showMessage (tr ("Initializing interface..."),
			Qt::AlignLeft | Qt::AlignBottom);

	InitializeInterface ();

	SplashScreen_->showMessage (tr ("Initializing core and plugins..."),
			Qt::AlignLeft | Qt::AlignBottom);
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

	Core::Instance ().SetReallyMainWindow (this);
	Core::Instance ().DelayedInit ();

	PluginManagerDialog_ = new PluginManagerDialog (this);

	SplashScreen_->showMessage (tr ("Initializing core and plugins..."),
			Qt::AlignLeft | Qt::AlignBottom);

	QAbstractItemModel *tasksModel = Core::Instance ().GetTasksModel ();
	Ui_.PluginsTasksTree_->setModel (tasksModel);

	Ui_.HistoryView_->setModel (Core::Instance ().GetHistoryModel ());
	Ui_.HistoryView_->sortByColumn (3, Qt::DescendingOrder);
	connect (Ui_.HistoryView_,
			SIGNAL (activated (const QModelIndex&)),
			this,
			SLOT (historyActivated (const QModelIndex&)));

	connect (Ui_.PluginsTasksTree_->selectionModel (),
			SIGNAL (selectionChanged (const QItemSelection&,
					const QItemSelection&)),
			this,
			SLOT (updatePanes (const QItemSelection&,
					const QItemSelection&)));

	QHeaderView *itemsHeader = Ui_.PluginsTasksTree_->header ();
	QFontMetrics fm = fontMetrics ();
	itemsHeader->resizeSection (0,
			fm.width ("Average download job or torrent name is just like this one maybe."));
	itemsHeader->resizeSection (1,
			fm.width ("State of the download."));
	itemsHeader->resizeSection (2,
			fm.width ("99.99% (1024.0 kb from 1024.0 kb at 1024.0 kb/s)"));

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

	updateIconSet ();

	setUpdatesEnabled (true);
	SplashScreen_->finish (this);
	show ();
	delete SplashScreen_;
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

void MainWindow::InitializeInterface ()
{
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

	QWidget *settings = new QWidget ();
	settings->addAction (Ui_.ActionSettings_);
	Ui_.MainTabWidget_->setCornerWidget (settings, Qt::TopRightCorner);
	Ui_.MainTabWidget_->setProperty ("TabIcons", "downloaders history");
	Ui_.ControlsDockWidget_->hide ();

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

	XmlSettingsDialog_ = new XmlSettingsDialog ();
	XmlSettingsDialog_->RegisterObject (XmlSettingsManager::Instance (),
			":/coresettings.xml");
	XmlSettingsManager::Instance ()->RegisterObject ("IconSet",
			this, "updateIconSet");

	SettingsSink_ = new SettingsSink (tr ("LeechCraft"), XmlSettingsDialog_);

	SetStatusBar ();
	ReadSettings ();

	FancyPopupManager_ = new FancyPopupManager (TrayIcon_, this);
	LogToolBox_ = new LogToolBox (this);

	connect (Ui_.HistoryView_,
			SIGNAL (deleteSelected (const QModelIndex&)),
			&Core::Instance (),
			SLOT (deleteSelectedHistory (const QModelIndex&)));

	QToolBar *mainBar = new QToolBar (this);
	mainBar->addAction (Ui_.ActionAddTask_);
	mainBar->addSeparator ();
	mainBar->addAction (Ui_.ActionSettings_);
	mainBar->addAction (Ui_.ActionPluginManager_);
	mainBar->addAction (Ui_.ActionLogger_);
	mainBar->addSeparator ();
	mainBar->addAction (Ui_.ActionFullscreenMode_);
	mainBar->addSeparator ();
	mainBar->addAction (Ui_.ActionQuit_);
	Ui_.ControlsLayout_->addWidget (mainBar);
}


void MainWindow::SetStatusBar ()
{
	QFontMetrics fm = fontMetrics ();
	int minSize = fm.width (Proxy::Instance ()->MakePrettySize (999) + tr ("/s    "));

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

	DSpeedGraph_ = new GraphWidget (Qt::green);
	DSpeedGraph_->setMinimumWidth (250);
	USpeedGraph_ = new GraphWidget (Qt::yellow);
	USpeedGraph_->setMinimumWidth (250);

	statusBar ()->addPermanentWidget (DownloadSpeed_);
	statusBar ()->addPermanentWidget (DSpeedGraph_);
	statusBar ()->addPermanentWidget (UploadSpeed_);
	statusBar ()->addPermanentWidget (USpeedGraph_);
	statusBar ()->addPermanentWidget (Clock_);
	Clock_->hide ();
}

void MainWindow::SetTrayIcon ()
{
	TrayIcon_ = new QSystemTrayIcon (QIcon (":/resources/images/mainapp.png"), this);

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
	SettingsSink_->show ();
}

void MainWindow::on_ActionQuit__triggered ()
{
	setUpdatesEnabled (false);
	WriteSettings ();
	Core::Instance ().Release ();

	TrayIcon_->hide ();
	delete TrayIcon_;
	qDebug () << "Releasing XmlSettingsManager";
	delete XmlSettingsDialog_;
	delete SettingsSink_;
	XmlSettingsManager::Instance ()->Release ();
	qDebug () << "Destroyed fine";

	qApp->quit ();
}

void MainWindow::on_ActionFullscreenMode__triggered (bool full)
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
		showNormal ();
}

void MainWindow::on_ActionLogger__triggered ()
{
	LogToolBox_->show ();
}

void MainWindow::updatePanes (const QItemSelection& newIndexes,
		const QItemSelection& oldIndexes)
{
	QModelIndex oldIndex, newIndex;
	if (oldIndexes.size ())
		oldIndex = oldIndexes.at (0).topLeft ();
	if (newIndexes.size ())
		newIndex = newIndexes.at (0).topLeft ();
	if (!newIndex.isValid ())
	{
		qDebug () << "invalidating";

		Core::Instance ().SetNewRow (newIndex);
		if (Ui_.ControlsLayout_->count () == 2)
		{
			Ui_.ControlsLayout_->takeAt (1)->widget ()->hide ();
			Ui_.ControlsDockWidget_->hide ();
		}
	}
	else if (oldIndex.isValid () &&
			Core::Instance ().SameModel (newIndex, oldIndex))
	{
		qDebug () << "setting new row";
		Core::Instance ().SetNewRow (newIndex);
	}
	else if (newIndex.isValid ())
	{
		if (oldIndex.isValid ())
		{
			qDebug () << "erasing older stuff";

			if (Ui_.ControlsLayout_->count () == 2)
			{
				Ui_.ControlsLayout_->takeAt (1)->widget ()->hide ();
				Ui_.ControlsDockWidget_->hide ();
			}
		}

		qDebug () << "inserting newer stuff" << newIndex;
		QWidget *controls = Core::Instance ()
					.GetControls (newIndex),
				*addiInfo = Core::Instance ()
					.GetAdditionalInfo (newIndex);

		Core::Instance ().SetNewRow (newIndex);
		
		Ui_.ControlsLayout_->addWidget (controls, 1);
		controls->show ();
		if (addiInfo)
		{
			Ui_.ControlsDockWidget_->setWidget (addiInfo);
			Ui_.ControlsDockWidget_->show ();
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

void MainWindow::updateClock ()
{
	Clock_->setText (QTime::currentTime ().toString ());
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
	SkinEngine::Instance ().UpdateIconSet (findChildren<QAction*> ());
	SkinEngine::Instance ().UpdateIconSet (findChildren<QTabWidget*> ());
}

void MainWindow::on_ActionPluginManager__triggered ()
{
	PluginManagerDialog_->show ();
}

void MainWindow::historyActivated (const QModelIndex& index)
{
	Core::Instance ().HistoryActivated (index.row ());
}

void MainWindow::handleLoadProgress (const QString& msg)
{
	SplashScreen_->
		showMessage (tr ("Initializing core and plugins...") + " " + msg,
				Qt::AlignLeft | Qt::AlignBottom);
}

