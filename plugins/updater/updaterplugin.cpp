#include <QtGui>
#include <settingsdialog/settingsdialog.h>
#include <plugininterface/proxy.h>
#include "updaterplugin.h"
#include "settingsmanager.h"
#include "globals.h"
#include "core.h"

void UpdaterPlugin::Init ()
{
	Core_ = new Core;
	connect (Core_, SIGNAL (gotFile (const QString&, const QString&, const QString&)), this, SLOT (addFile (const QString&, const QString&, const QString)));
	connect (Core_, SIGNAL (error (const QString&)), this, SLOT (handleError (const QString&)));
	Core_->start (QThread::LowestPriority);
	setWindowTitle (tr ("Updater"));
	setWindowIcon (GetIcon ());
	IsShown_ = false;
	SaveChangesScheduled_ = false;

	SettingsDialog_ = new SettingsDialog ();
	SettingsDialog_->RegisterObject (SettingsManager::Instance ());

	SetupInterface ();

	ReadSettings ();
}

UpdaterPlugin::~UpdaterPlugin ()
{
}

QString UpdaterPlugin::GetName () const
{
	return Globals::Name;
}

QString UpdaterPlugin::GetInfo () const
{
	return tr ("Simple updater");
}

QString UpdaterPlugin::GetStatusbarMessage () const
{
	return tr ("");
}

IInfo& UpdaterPlugin::SetID (IInfo::ID_t id)
{
	ID_ = id;
	return *this;
}

IInfo::ID_t UpdaterPlugin::GetID () const
{
	return ID_;
}

QStringList UpdaterPlugin::Provides () const
{
	return QStringList ("update");
}

QStringList UpdaterPlugin::Needs () const
{
	return (QStringList ("http") << "ftp");
}

QStringList UpdaterPlugin::Uses () const
{
	return QStringList ();
}

void UpdaterPlugin::SetProvider (QObject* provider, const QString& feature)
{
	Core_->SetProvider (provider, feature);
}

void UpdaterPlugin::Release ()
{
	saveSettings ();
	delete Core_;
}

QIcon UpdaterPlugin::GetIcon () const
{
	return QIcon (":/resources/images/updater.png");
}

void UpdaterPlugin::SetParent (QWidget *parent)
{
	setParent (parent);
}

void UpdaterPlugin::ShowWindow ()
{
	IsShown_ = 1 - IsShown_;
	IsShown_ ? show () : hide ();
}

void UpdaterPlugin::ShowBalloonTip ()
{
}

uint UpdaterPlugin::GetVersion () const
{
	return QDateTime (QDate (2007, 11, 30), QTime (11, 11)).toTime_t ();
}

void UpdaterPlugin::SetupInterface ()
{
	SetupMainWidget ();
	SetupToolbars ();
	SetupActions ();
	SetupMenus ();
}

void UpdaterPlugin::SetupMainWidget ()
{
	Updates_ = new QTreeWidget (this);
	setCentralWidget (Updates_);
	Updates_->setHeaderLabels (QStringList (tr ("Plugin")) << tr ("Location"));
	Updates_->setEditTriggers (QAbstractItemView::NoEditTriggers);
	Updates_->setSelectionMode (QAbstractItemView::NoSelection);
	Updates_->setAlternatingRowColors (true);
	Updates_->header ()->setStretchLastSection (true);
	Updates_->header ()->setHighlightSections (false);
	Updates_->header ()->setDefaultAlignment (Qt::AlignLeft);
}

void UpdaterPlugin::SetupToolbars ()
{
	MainToolbar_ = addToolBar (tr ("Main"));
	MainToolbar_->setIconSize (QSize (16, 16));
}

void UpdaterPlugin::SetupActions ()
{
	CheckForUpdates_ = MainToolbar_->addAction (QIcon (":/resources/images/check.png"), tr ("Check for updates"), Core_, SLOT (checkForUpdates ()));
	DownloadUpdates_ = MainToolbar_->addAction (QIcon (":/resources/images/download.png"), tr ("Download updates"), Core_, SLOT (downloadUpdates ()));
	MainToolbar_->addSeparator ();
	Settings_ = MainToolbar_->addAction (QIcon (":/resources/images/preferences.png"), tr ("Settings..."), this, SLOT (showSettings ()));
}

void UpdaterPlugin::SetupMenus ()
{
	QMenu *tools = menuBar ()->addMenu (tr ("&Tools"));
	tools->addAction (CheckForUpdates_);
	tools->addSeparator ();
	tools->addAction (Settings_);
}

void UpdaterPlugin::ReadSettings ()
{
    QSettings settings (Proxy::Instance ()->GetOrganizationName (), Proxy::Instance ()->GetApplicationName ());
    settings.beginGroup (GetName ());
    settings.beginGroup ("geometry");
    resize (settings.value ("size", QSize (400, 300)).toSize ());
    move   (settings.value ("pos",  QPoint (10, 10)).toPoint ());
	settings.endGroup ();
	settings.endGroup ();
}

void UpdaterPlugin::saveSettings ()
{
	SaveChangesScheduled_ = false;
    QSettings settings (Proxy::Instance ()->GetOrganizationName (), Proxy::Instance ()->GetApplicationName ());
    settings.beginGroup (GetName ());
    settings.beginGroup ("geometry");
    settings.setValue ("size", size ());
    settings.setValue ("pos", pos ());
	settings.endGroup ();
	settings.endGroup ();
}

void UpdaterPlugin::showSettings ()
{
	SettingsDialog_->show ();
	SettingsDialog_->setWindowTitle (windowTitle () + tr (": Preferences"));
}

void UpdaterPlugin::addFile (const QString& name, const QString& loc, const QString& descr)
{
	QTreeWidgetItem *item = new QTreeWidgetItem (Updates_);
	item->setText (ColumnName_, name);
	item->setText (ColumnLocation_, loc);

	QTreeWidgetItem *descItem = new QTreeWidgetItem (item);
	descItem->setFirstColumnSpanned (true);
	descItem->setText (0, descr);
}

void UpdaterPlugin::handleError (const QString& error)
{
	qDebug () << error;
	QMessageBox::warning (this, tr ("Error!"), error);
}

Q_EXPORT_PLUGIN2 (leechcraft_updater, UpdaterPlugin);

