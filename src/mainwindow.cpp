#include <QtGui/QtGui>
#include <QMutex>
#include <iostream>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "plugininterface/proxy.h"
#include "plugininterface/graphwidget.h"
#include "exceptions/notimplemented.h"
#include "mainwindow.h"
#include "view.h"
#include "core.h"
#include "plugininfo.h"
#include "pluginlisttablewidgeticon.h"
#include "changelogdialog.h"
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

	connect (Ui_->ActionAddTask_, SIGNAL (triggered ()), this, SLOT (addJob ()));
	connect (Ui_->ActionQuit_, SIGNAL (triggered ()), qApp, SLOT (quit ()));
	connect (Ui_->ActionSettings_, SIGNAL (triggered ()), this, SLOT (showSettings ()));
	connect (Ui_->ActionAboutQt_, SIGNAL (triggered ()), qApp, SLOT (aboutQt ()));
	connect (Ui_->ActionAboutLeechCraft_, SIGNAL (triggered ()), this, SLOT (showAboutInfo ()));

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
    Proxy::Instance ()->SetMainWindow (this);

    splash.showMessage (tr ("Initializing core and plugins..."));
    connect (&Core::Instance (),
			SIGNAL (downloadFinished (const QString&)),
			this,
			SLOT (handleDownloadFinished (const QString&)));
    connect (qApp, SIGNAL (aboutToQuit ()), this, SLOT (cleanUp ()));
	Core::Instance ().SetReallyMainWindow (this);

	Core::Instance ().DelayedInit ();

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
}

QMenu* MainWindow::GetRootPluginsMenu () const
{
    return Ui_->PluginsMenu_;
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

void MainWindow::SetTrayIcon ()
{
	QMenu *iconMenu = new QMenu;
	iconMenu->addAction (tr ("Show/hide main"), this, SLOT (showHideMain ()));
	iconMenu->addAction (tr ("Hide all"), this, SLOT (hideAll ()));
	iconMenu->addSeparator ();
	iconMenu->addAction (Ui_->ActionAddTask_);
	iconMenu->addSeparator ();
	TrayPluginsMenu_ = iconMenu->addMenu (tr ("Plugins"));
	iconMenu->addSeparator ();
	iconMenu->addAction (tr ("Quit"), qApp, SLOT (quit ()));

	TrayIcon_ = new QSystemTrayIcon (QIcon (":/resources/images/mainapp.png"), this);
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

void MainWindow::showAboutInfo ()
{
    QMessageBox::information (this, tr ("Information"), tr ("<img src=\":/resources/images/mainapp.png\" /><h1>LeechCraft 0.3.0_pre</h1>"
                "LeechCraft is a cross-platform extensible download manager. Currently it offers "
                "full-featured BitTorrent client, feed reader, HTTP/FTP plugin, Remote access "
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

void MainWindow::hideAll ()
{
	Core::Instance ().HideAll ();
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
        TrayIcon_->showMessage (tr ("Download finished"), string, QSystemTrayIcon::Information, XmlSettingsManager::Instance ()->property ("FinishedDownloadMessageTimeout").toInt () * 1000);
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
    TrayIcon_->hide ();
    WriteSettings ();
	Core::Instance ().Release ();
    qDebug () << "Releasing XmlSettingsManager";
    XmlSettingsManager::Instance ()->Release ();
	Ui_ = 0;
    qDebug () << "Destroyed fine";
	delete Ui_;
}

};

