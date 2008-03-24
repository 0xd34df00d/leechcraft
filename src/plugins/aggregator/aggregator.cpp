#include <QMessageBox>
#include <QtDebug>
#include <QSortFilterProxyModel>
#include <QHeaderView>
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
    connect (&Core::Instance (), SIGNAL (showDownloadMessage (const QString&)), this, SIGNAL (downloadFinished (const QString&)));

    XmlSettingsDialog_ = new XmlSettingsDialog (this);
    XmlSettingsDialog_->RegisterObject (XmlSettingsManager::Instance (), ":/aggregatorsettings.xml");

    Core::Instance ().DoDelayedInit ();
    ItemsFilterModel_ = new QSortFilterProxyModel (this);
    ItemsFilterModel_->setSourceModel (&Core::Instance ());
    ItemsFilterModel_->setFilterKeyColumn (0);
    Ui_.Items_->setModel (ItemsFilterModel_);
    connect (&Core::Instance (), SIGNAL (dataChanged (const QModelIndex&, const QModelIndex&)), ItemsFilterModel_, SLOT (invalidate ()));
    connect (Ui_.FixedStringSearch_, SIGNAL (textChanged (const QString&)), ItemsFilterModel_, SLOT (setFilterFixedString (const QString&)));
    connect (Ui_.WildcardSearch_, SIGNAL (textChanged (const QString&)), ItemsFilterModel_, SLOT (setFilterWildcard (const QString&)));
    connect (Ui_.RegexpSearch_, SIGNAL (textChanged (const QString&)), ItemsFilterModel_, SLOT (setFilterRegExp (const QString&)));

    QHeaderView *itemsHeader = Ui_.Items_->header ();
    QFontMetrics fm = fontMetrics ();
    itemsHeader->resizeSection (1, fm.width ("_99 Mar 9999 99:99:99_"));

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
    return tr ("RSS 2.0, Atom 1.0 feed reader.");
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

void Aggregator::on_ActionRemoveFeed__triggered ()
{
    Core::Instance ().RemoveFeed (Ui_.Feeds_->selectionModel ()->currentIndex ());
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

