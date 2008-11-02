#include <iostream>
#include <algorithm>
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
#include "pluginmanagerdialog.h"
#include "fancypopupmanager.h"
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

		Ui_->AddTaskButton_->setDefaultAction (Ui_->ActionAddTask_);

		Ui_->ActionAddTask_->setProperty ("ActionIcon", "addjob");
		Ui_->ActionSettings_->setProperty ("ActionIcon", "settings");

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

		connect (Ui_->HistoryFilterCaseSensitivity_,
				SIGNAL (stateChanged (int)),
				this,
				SLOT (historyFilterParametersChanged ()));
		connect (Ui_->HistoryFilterLine_,
				SIGNAL (textEdited (const QString&)),
				this,
				SLOT (historyFilterParametersChanged ()));
		connect (Ui_->HistoryFilterType_,
				SIGNAL (currentIndexChanged (int)),
				this,
				SLOT (historyFilterParametersChanged ()));

		SetTrayIcon ();

		FancyPopupManager_ = new FancyPopupManager (TrayIcon_, this);

		XmlSettingsDialog_ = new XmlSettingsDialog (this);
		XmlSettingsDialog_->RegisterObject (XmlSettingsManager::Instance (),
				":/coresettings.xml");
		XmlSettingsManager::Instance ()->RegisterObject ("AggregateJobs",
				this, "handleAggregateJobsChange");
		XmlSettingsManager::Instance ()->RegisterObject ("IconSet",
				this, "updateIconsSet");

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

		PluginManagerDialog_ = new PluginManagerDialog (this);

		splash.showMessage (tr ("Initializing core and plugins..."));
		connect (&Core::Instance (),
				SIGNAL (downloadFinished (const QString&)),
				this,
				SLOT (handleDownloadFinished (const QString&)));
		connect (qApp, SIGNAL (aboutToQuit ()), this, SLOT (cleanUp ()));

		Core::Instance ().SetReallyMainWindow (this);
		Core::Instance ().DelayedInit ();

		QAbstractItemModel *tasksModel = Core::Instance ().GetTasksModel ();
		Ui_->PluginsTasksTree_->setModel (tasksModel);

		Ui_->HistoryView_->setModel (Core::Instance ().GetHistoryModel ());
		connect (Ui_->HistoryView_,
				SIGNAL (activated (const QModelIndex&)),
				this,
				SLOT (historyActivated (const QModelIndex&)));

		connect (Ui_->PluginsTasksTree_->selectionModel (),
				SIGNAL (currentRowChanged (const QModelIndex&,
						const QModelIndex&)),
				this,
				SLOT (updatePanes (const QModelIndex&,
						const QModelIndex&)));

		QHeaderView *itemsHeader = Ui_->PluginsTasksTree_->header ();
		QFontMetrics fm = fontMetrics ();
		itemsHeader->resizeSection (0,
				fm.width ("Average download job or torrent name is just like this one maybe."));
		itemsHeader->resizeSection (1,
				fm.width ("State of the download."));
		itemsHeader->resizeSection (2,
				fm.width ("99.99% (1234.56 kb from 2345.67 kb)"));
		itemsHeader->resizeSection (3,
				fm.width (" 1234.56 kb/s "));

		itemsHeader = Ui_->HistoryView_->header ();
		itemsHeader->resize (0,
				fm.width ("Average filename or torrent name is about this width or something."));
		itemsHeader->resize (1,
				fm.width ("/home/this/is/some/path/for/downloaded/file/"));
		itemsHeader->resize (2,
				fm.width (" 1234.56 kb "));
		itemsHeader->resize (3,
				fm.width (QDateTime::currentDateTime ().toString ()));

		QTimer *speedUpd = new QTimer (this);
		speedUpd->setInterval (1000);
		connect (speedUpd, SIGNAL (timeout ()), this, SLOT (updateSpeedIndicators ()));
		speedUpd->start ();
		qApp->setQuitOnLastWindowClosed (false);

		updateIconsSet ();

		splash.finish (this);
		show ();
	}

	MainWindow::~MainWindow ()
	{
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

	void MainWindow::updatePanes (const QModelIndex& newIndex,
			const QModelIndex& oldIndex)
	{
		Core::Instance ().SetNewRow (newIndex);

		if (Core::Instance ().SameModel (newIndex, oldIndex))
			return;

		if (Ui_->PluginsStuff_->count () == 4)
		{
			Ui_->PluginsStuff_->takeAt (3)->widget ()->hide ();
			Ui_->PluginsStuff_->takeAt (1)->widget ()->hide ();
		}

		if (newIndex.isValid ())
		{
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
		if (adder.exec () != QDialog::Accepted)
			return;

		QString name = adder.GetString ();
		if (!name.isEmpty ())
			Core::Instance ().TryToAddJob (name, adder.GetWhere ());
	}

	void MainWindow::handleDownloadFinished (const QString& string)
	{
		if (XmlSettingsManager::Instance ()->property ("ShowFinishedDownloadMessages").toBool ())
			FancyPopupManager_->ShowMessage (string);
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
		delete Ui_;
		Ui_ = 0;
		WriteSettings ();
		Core::Instance ().Release ();

		TrayIcon_->hide ();
		delete TrayIcon_;
		qDebug () << "Releasing XmlSettingsManager";
		delete XmlSettingsDialog_;
		XmlSettingsManager::Instance ()->Release ();
		qDebug () << "Destroyed fine";
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
				ft, caseSensitivity);
	}

	void MainWindow::historyFilterParametersChanged ()
	{
		Core::FilterType ft;
		switch (Ui_->HistoryFilterType_->currentIndex ())
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
					<< Ui_->HistoryFilterType_->currentIndex ();
				return;
		}

		bool caseSensitivity =
			(Ui_->HistoryFilterCaseSensitivity_->checkState () == Qt::Checked);
		Core::Instance ().UpdateFiltering (Ui_->HistoryFilterLine_->text (),
				ft, caseSensitivity, true);
	}

	namespace
	{
		std::vector<int> GetDirForBase (const QString& base,
				const QString& iconSet)
		{
			QDir baseDir (base);
			baseDir.cd (iconSet);
			std::vector<int> numbers;
			QStringList entries = baseDir.entryList ();
			for (QStringList::const_iterator i = entries.begin (),
					end = entries.end (); i != end; ++i)
			{
				QStringList splitted = i->split ('x');
				if (splitted.size () != 2)
					continue;

				numbers.push_back (splitted.at (0).toInt ());
			}

			std::sort (numbers.begin (), numbers.end ());
			return numbers;
		}
	};

	void MainWindow::updateIconsSet ()
	{
		QString iconSet = XmlSettingsManager::Instance ()->
			property ("IconSet").toString ();
		QMap<QString, QString> iconName2Path;
		QMap<QString, QString> iconName2FileName;
#if defined (Q_OS_UNIX)
		QDir dir ("/usr/share/leechcraft/icons");
		if (!dir.exists ())
			dir = "/usr/local/share/leechcraft/icons";
#elif defined (Q_OS_WIN32)
		QDir dir = QApplication::applicationDirPath ();
		dir.cd ("icons");
#endif
		if (dir.exists (iconSet + ".mapping"))
		{
			QFile mappingFile (dir.filePath (iconSet + ".mapping"));
			if (mappingFile.open (QIODevice::ReadOnly))
			{
				QByteArray lineData = mappingFile.readLine ();
				while (!lineData.isEmpty ())
				{
					QStringList pair = QString::fromUtf8 (lineData)
						.split (' ', QString::SkipEmptyParts);
					if (pair.size () != 2)
						continue;

					iconName2FileName [pair.at (0).simplified ()] = pair.at (1).simplified ();

					lineData = mappingFile.readLine ();
				}
			}
		}

#if defined (Q_OS_UNIX)
		std::vector<int> numbers = GetDirForBase ("/usr/share/icons", iconSet);
		QDir baseDir ("/usr/share/icons");
		baseDir.cd (iconSet);
		for (std::vector<int>::const_iterator i = numbers.begin (),
				end = numbers.end (); i != end; ++i)
		{
			QDir current = baseDir;
			QString number = QString::number (*i);
			current.cd (number + 'x' + number);
			current.cd ("actions");
			QFileInfoList infos =
				current.entryInfoList (QStringList ("lc_*.png"),
						QDir::Files | QDir::Readable);

			for (QFileInfoList::const_iterator j = infos.begin (),
					infoEnd = infos.end (); j != infoEnd; ++j)
				iconName2Path [j->fileName ()] = j->absoluteFilePath ();
		}

		baseDir = QDir ("/usr/local/share/icons");
		numbers = GetDirForBase ("/usr/local/share/icons", iconSet);
		baseDir.cd (iconSet);
		for (std::vector<int>::const_iterator i = numbers.begin (),
				end = numbers.end (); i != end; ++i)
		{
			QDir current = baseDir;
			QString number = QString::number (*i);
			current.cd (number + 'x' + number);
			current.cd ("actions");
			QFileInfoList infos =
				current.entryInfoList (QStringList ("lc_*.png"),
						QDir::Files | QDir::Readable);

			for (QFileInfoList::const_iterator j = infos.begin (),
					infoEnd = infos.end (); j != infoEnd; ++j)
				iconName2Path [j->fileName ()] = j->absoluteFilePath ();
		}
#elif defined (Q_OS_WIN32)
		QDir baseDir = QApplication::applicationDirPath ();
		baseDir.cd ("icons");
		std::vector<int> numbers = GetDirForBase (baseDir.absolutePath (), iconSet);
		baseDir.cd (iconSet);
		for (std::vector<int>::const_iterator i = numbers.begin (),
				end = numbers.end (); i != end; ++i)
		{
			QDir current = baseDir;
			QString number = QString::number (*i);
			current.cd (number + 'x' + number);
			current.cd ("actions");
			QFileInfoList infos =
				current.entryInfoList (QStringList ("lc_*.png"),
						QDir::Files | QDir::Readable);

			for (QFileInfoList::const_iterator j = infos.begin (),
					infoEnd = infos.end (); j != infoEnd; ++j)
				iconName2Path [j->fileName ()] = j->absoluteFilePath ();
		}
#endif

		QList<QAction*> actions = findChildren<QAction*> ();
		for (QList<QAction*>::iterator i = actions.begin (),
				end = actions.end (); i != end; ++i)
		{
			if (!(*i)->property ("ActionIcon").isValid ())
				continue;
			QString actionIcon = (*i)->property ("ActionIcon").toString ();
			QString actionIconOff = (*i)->property ("ActionIconOff").toString ();
			QString icon;
			if (iconName2FileName.contains (actionIcon))
				icon = iconName2FileName [actionIcon] + ".png";
			else
				icon = QString ("lc_") + actionIcon + ".png";

			QIcon iconEntity;
			iconEntity.addPixmap (QPixmap (iconName2Path [icon]),
					QIcon::Normal,
					QIcon::On);

			if (actionIconOff.size ())
			{
				QString offIcon;
				if (iconName2FileName.contains (actionIconOff))
					icon = iconName2FileName [actionIconOff] + ".png";
				else
					icon = QString ("lc_") + actionIconOff + ".png";
				iconEntity.addPixmap (icon, QIcon::Normal, QIcon::Off);
			}
			
			(*i)->setIcon (iconEntity);
		}
	}

	void MainWindow::on_ActionPluginManager__triggered ()
	{
		PluginManagerDialog_->show ();
	}

	void MainWindow::historyActivated (const QModelIndex& index)
	{
		Core::Instance ().HistoryActivated (index.row ());
	}
};

