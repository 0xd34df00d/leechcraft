#include <QMessageBox>
#include <QtDebug>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "aggregator.h"
#include "core.h"
#include "addfeed.h"
#include "xmlsettingsmanager.h"

void Aggregator::Init ()
{
    Ui_.setupUi (this);
    IsShown_ = false;

    Plugins_->addAction (Ui_.ActionAddFeed_);
    connect (&Core::Instance (), SIGNAL (error (const QString&)), this, SLOT (showError (const QString&)));

    XmlSettingsDialog_ = new XmlSettingsDialog (this);
    XmlSettingsDialog_->RegisterObject (XmlSettingsManager::Instance (), ":/aggregatorsettings.xml");
    Core::Instance ().DoDelayedInit ();

    Ui_.Items_->setModel (&Core::Instance ());
    Ui_.Feeds_->setModel (Core::Instance ().GetChannelsModel ());
    connect (Ui_.Items_->selectionModel (), SIGNAL (currentChanged (const QModelIndex&, const QModelIndex&)), this, SLOT (currentItemChanged (const QModelIndex&)));
    connect (Ui_.Feeds_->selectionModel (), SIGNAL (currentChanged (const QModelIndex&, const QModelIndex&)), &Core::Instance (), SLOT (currentChannelChanged (const QModelIndex&)));
    connect (Ui_.ActionUpdateFeeds_, SIGNAL (triggered ()), &Core::Instance (), SLOT (updateFeeds ()));
}

void Aggregator::Release ()
{
    Core::Instance ().Release ();
    delete XmlSettingsDialog_;
}

QString Aggregator::GetName () const
{
    return "Aggregator";
}

QString Aggregator::GetInfo () const
{
    return tr ("RSS2.0/Atom1.0 feed reader.");
}

QString Aggregator::GetStatusbarMessage () const
{
    return QString ();
}

IInfo& Aggregator::SetID (long unsigned int id)
{
    ID_ = id;
    return *this;
}

unsigned long int Aggregator::GetID () const
{
    return ID_;
}

QStringList Aggregator::Provides () const
{
    return QStringList ("rss");
}

QStringList Aggregator::Needs () const
{
    return QStringList ("http");
}

QStringList Aggregator::Uses () const
{
    return QStringList ();
}

void Aggregator::SetProvider (QObject* object, const QString& feature)
{
    Core::Instance ().SetProvider (object, feature);
}

void Aggregator::PushMainWindowExternals (const MainWindowExternals& externals)
{
    Plugins_ = externals.RootMenu_->addMenu ("&Aggregator");
}

QIcon Aggregator::GetIcon () const
{
    return windowIcon ();
}

void Aggregator::SetParent (QWidget *parent)
{
    setParent (parent);
}

void Aggregator::ShowWindow ()
{
    IsShown_ = 1 - IsShown_;
    IsShown_ ? show () : hide ();
}

void Aggregator::ShowBalloonTip ()
{
}

void Aggregator::closeEvent (QCloseEvent*)
{
    IsShown_ = false;
}

void Aggregator::showError (const QString& msg)
{
    qDebug () << Q_FUNC_INFO << msg;
    QMessageBox::warning (0, tr ("Error"), msg);
}

void Aggregator::on_ActionAddFeed__triggered ()
{
    AddFeed af;
    if (af.exec () == QDialog::Accepted)
        Core::Instance ().AddFeed (af.GetURL ());
}

void Aggregator::on_ActionPreferences__triggered ()
{
    XmlSettingsDialog_->show ();
    XmlSettingsDialog_->setWindowTitle (windowTitle () + tr (": Preferences"));
}

void Aggregator::on_Items__activated (const QModelIndex& index)
{
    Core::Instance ().Activated (index);
}

void Aggregator::on_Items__doubleClicked (const QModelIndex& index)
{
    Core::Instance ().Activated (index);
}

void Aggregator::currentItemChanged (const QModelIndex& index)
{
    Ui_.ItemView_->setHtml (Core::Instance ().GetDescription (index));
}

Q_EXPORT_PLUGIN2 (leechcraft_aggregator, Aggregator);

