#include <iostream>
#include <QtGui/QtGui>
#include <QMutex>
#include <QModelIndex>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "plugininterface/proxy.h"
#include "plugininterface/graphwidget.h"
#include "mainwindow.h"
#include "view.h"
#include "core.h"
#include "commonjobadder.h"
#include "xmlsettingsmanager.h"
#include "ui_leechcraft.h"

namespace Main
{

	MainWindow::MainWindow (QWidget *parent, Qt::WFlags flags)
	: QMainWindow (parent, flags)
	, IsShown_ (true)
	{
		QSplashScreen splash (QPixmap (":/resources/images/splashscreen.png"),
				Qt::WindowStaysOnTopHint);
		splash.show ();
		splash.showMessage (tr ("Initializing interface..."));

		Ui_ = new Ui::LeechCraft;
		Ui_->setupUi (this);
		Ui_->MainSplitter_->setStretchFactor (1, 4);

		connect (Ui_->ActionAddTask_,
				SIGNAL (triggered ()),
				this,
				SLOT (addJob ()));
		connect (Ui_->ActionQuit_,
				SIGNAL (triggered ()),
				qApp,
				SLOT (quit ()));
		connect (Ui_->ActionSettings_,
				SIGNAL (triggered ()),
				this,
				SLOT (showSettings ()));
		connect (Ui_->ActionAboutQt_,
				SIGNAL (triggered ()),
				qApp,
				SLOT (aboutQt ()));
		connect (Ui_->ActionAboutLeechCraft_,
				SIGNAL (triggered ()),
				this,
				SLOT (showAboutInfo ()));
		
		connect (Ui_->FilterCaseSensitivity_,
				SIGNAL (stateChanged (int)),
				this,
				SLOT (filterParametersChanged ()));
		connect (Ui_->FilterLine_,
				SIGNAL (textEdited (const QString&)),
				this,
				SLOT (filterParametersChanged ()));
		connect (Ui_->FilterType_,
				SIGNAL (currentIndexChanged (int)),
				this,
				SLOT (filterParametersChanged ()));

		SetTrayIcon ();

		XmlSettingsDialog_ = new XmlSettingsDialog (this);
		XmlSettingsDialog_->RegisterObject (XmlSettingsManager::Instance (), ":/coresettings.xml");
		XmlSettingsManager::Instance ()->RegisterObject ("AggregateJobs",
				this, "handleAggregateJobsChange");

		DownloadSpeed_ = new QLabel;
		DownloadSpeed_->setText ("0");
		DownloadSpeed_->setMinimumWidth (70);
		DownloadSpeed_->setAlignment (Qt::AlignRight);
		UploadSpeed_ = new QLabel;
		UploadSpeed_->setText ("0");
		UploadSpeed_->setMinimumWidth (70);
		UploadSpeed_->setAlignment (Qt::AlignRight);

		DSpeedGraph_ = new GraphWidget (Qt::green);
		DSpeedGraph_->setMinimumWidth (100);
		USpeedGraph_ = new GraphWidget (Qt::yellow);
		USpeedGraph_->setMinimumWidth (100);

		statusBar ()->addPermanentWidget (DSpeedGraph_);
		statusBar ()->addPermanentWidget (USpeedGraph_);
		statusBar ()->addPermanentWidget (DownloadSpeed_);
		statusBar ()->addPermanentWidget (UploadSpeed_);
		ReadSettings ();

		splash.showMessage (tr ("Initializing core and plugins..."));
		connect (&Core::Instance (),
				SIGNAL (downloadFinished (const QString&)),
				this,
				SLOT (handleDownloadFinished (const QString&)));
		connect (qApp, SIGNAL (aboutToQuit ()), this, SLOT (cleanUp ()));
		Core::Instance ().SetReallyMainWindow (this);

		Core::Instance ().DelayedInit ();

		Ui_->PluginsTree_->setModel (Core::Instance ().GetPluginsModel ());
		QAbstractItemModel *tasksModel = Core::Instance ().GetTasksModel ();
		Ui_->PluginsTasksTree_->setModel (tasksModel);

		connect (Ui_->PluginsTasksTree_->selectionModel (),
				SIGNAL (currentRowChanged (const QModelIndex&, const QModelIndex&)),
				this,
				SLOT (updatePanes (const QModelIndex&)));
		connect (Ui_->PluginsTasksTree_,
				SIGNAL (clicked (const QModelIndex&)),
				this,
				SLOT (updatePanes (const QModelIndex&)));

		QTimer *speedUpd = new QTimer (this);
		speedUpd->setInterval (1000);
		connect (speedUpd, SIGNAL (timeout ()), this, SLOT (updateSpeedIndicators ()));
		speedUpd->start ();
		qApp->setQuitOnLastWindowClosed (false);

		splash.finish (this);
		show ();
	}

	MainWindow::~MainWindow ()
	{
		delete Ui_;
	}

	QMenu* MainWindow::GetRootPluginsMenu () const
	{
		return Ui_->PluginsMenu_;
	}

	QModelIndexList MainWindow::GetSelectedRows () const
	{
		return Ui_->PluginsTasksTree_->selectionModel ()->selectedRows ();
	}

	QTabWidget* MainWindow::GetTabWidget () const
	{
		return Ui_->MainTabWidget_;
	}

	void MainWindow::catchError (QString message)
	{
		QMessageBox::critical (this, tr ("Error"), message);
	}

	void MainWindow::updatePanes (const QModelIndex& newIndex)
	{
		if (Ui_->PluginsStuff_->count () == 4)
		{
			Ui_->PluginsStuff_->takeAt (3)->widget ()->hide ();
			Ui_->PluginsStuff_->takeAt (1)->widget ()->hide ();
		}

		if (newIndex.isValid ())
		{
			Core::Instance ().SetNewRow (newIndex);

			QWidget *controls = Core::Instance ()
						.GetControls (newIndex),
					*addiInfo = Core::Instance ()
						.GetAdditionalInfo (newIndex);
			Ui_->PluginsStuff_->insertWidget (1, controls);
			Ui_->PluginsStuff_->addWidget (addiInfo);
			controls->show ();
			addiInfo->show ();
		}
	}

	void MainWindow::closeEvent (QCloseEvent *e)
	{
		e->ignore ();
		hide ();
		IsShown_ = false;
	}

	void MainWindow::SetTrayIcon ()
	{
		TrayIcon_ = new QSystemTrayIcon (QIcon (":/resources/images/mainapp.png"), this);

		QMenu *iconMenu = new QMenu (this);
		iconMenu->addAction (tr ("Show/hide"), this, SLOT (showHideMain ()));
		iconMenu->addSeparator ();
		iconMenu->addAction (Ui_->ActionAddTask_);
		iconMenu->addSeparator ();
		iconMenu->addAction (tr ("Quit"), qApp, SLOT (quit ()));

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
		settings.value ("maximized").toBool () ? showMaximized () : showNormal ();
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

	void MainWindow::updateSpeedIndicators ()
	{
		QPair<qint64, qint64> speeds = Core::Instance ().GetSpeeds ();

		DownloadSpeed_->setText (Proxy::Instance ()->MakePrettySize (speeds.first) + tr ("/s"));
		UploadSpeed_->setText (Proxy::Instance ()->MakePrettySize (speeds.second) + tr ("/s"));
		DSpeedGraph_->PushSpeed (speeds.first);
		USpeedGraph_->PushSpeed (speeds.second);
	}

	void MainWindow::showAboutInfo ()
	{
		QMessageBox::information (this, tr ("Information"), tr ("<img src=\":/resources/images/mainapp.png\" /><h1>LeechCraft 0.3.0_pre</h1>"
					"LeechCraft is a cross-platform extensible download manager. Currently it offers "
					"full-featured BitTorrent client, feed reader, HTTP support, Remote access "
					"and much more. It also aims to be resource-efficient working quite well on "
					"even old computers.<br /><br />Here are some useful links for you:<br />"
					"<a href=\"http://bugs.deviant-soft.ws\">Bugtracker and feature request tracker</a><br />"
					"<a href=\"http://sourceforge.net/project/showfiles.php?group_id=161819\">Latest file releases</a><br />"
					"<a href=\"http://deviant-soft.ws\">LeechCraft's Site</a><br />"
					"<a href=\"http://sourceforge.net/projects/leechcraft\">LeechCraft's site at sourceforge.net</a><br />"));
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

	void MainWindow::addJob ()
	{
		CommonJobAdder adder (this);
		if (adder.exec () == QDialog::Accepted)
		{
			QString name = adder.GetString ();
			if (!name.isEmpty ())
				Core::Instance ().TryToAddJob (name, adder.GetWhere ());
		}
	}

	void MainWindow::handleDownloadFinished (const QString& string)
	{
		if (XmlSettingsManager::Instance ()->property ("ShowFinishedDownloadMessages").toBool ())
			TrayIcon_->showMessage (tr ("Download finished"),
					string,
					QSystemTrayIcon::Information,
					XmlSettingsManager::Instance ()->property ("FinishedDownloadMessageTimeout").toInt () * 1000);
	}

	void MainWindow::showSettings ()
	{
		XmlSettingsDialog_->show ();
		XmlSettingsDialog_->setWindowTitle (windowTitle () + tr (": Preferences"));
	}

	void MainWindow::handleAggregateJobsChange ()
	{
		QSplitter *split = qobject_cast<QSplitter*> (centralWidget ());
		if (XmlSettingsManager::Instance ()->property ("AggregateJobs").toBool ())
			split->widget (1)->show ();
		else
			split->widget (1)->hide ();
	}

	void MainWindow::cleanUp ()
	{
		WriteSettings ();
		Core::Instance ().Release ();

		TrayIcon_->hide ();
		delete TrayIcon_;
		qDebug () << "Releasing XmlSettingsManager";
		delete XmlSettingsDialog_;
		XmlSettingsManager::Instance ()->Release ();
		delete Ui_;
		Ui_ = 0;
		qDebug () << "Destroyed fine";
	}

	void MainWindow::on_PluginsTree__activated (const QModelIndex& index)
	{
		Core::Instance ().Activated (index);
	}

	void MainWindow::filterParametersChanged ()
	{
		Core::FilterType ft;
		switch (Ui_->FilterType_->currentIndex ())
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
					<< Ui_->FilterType_->currentIndex ();
				return;
		}

		bool caseSensitivity = (Ui_->FilterCaseSensitivity_->checkState () == Qt::Checked);
		Core::Instance ().UpdateFiltering (Ui_->FilterLine_->text (),
				ft,
				caseSensitivity);
	}
};

