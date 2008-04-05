#include <QMessageBox>
#include <QtDebug>
#include <QSortFilterProxyModel>
#include <QHeaderView>
#include <QCompleter>
#include <QTranslator>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "aggregator.h"
#include "core.h"
#include "addfeed.h"
#include "channelsfiltermodel.h"
#include "xmlsettingsmanager.h"
#include "tagscompletionmodel.h"
#include "tagscompleter.h"

void Aggregator::Init ()
{
    QTranslator *transl = new QTranslator (this);
    QString localeName = QLocale::system ().name ();
    transl->load (QString (":/leechcraft_aggregator_") + localeName);
    qApp->installTranslator (transl);

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
    ItemsFilterModel_->setDynamicSortFilter (true);
    Ui_.Items_->setModel (ItemsFilterModel_);
    Ui_.Items_->addAction (Ui_.ActionMarkItemAsUnread_);
    Ui_.Items_->setContextMenuPolicy (Qt::ActionsContextMenu);
    connect (Ui_.FixedStringSearch_, SIGNAL (textChanged (const QString&)), ItemsFilterModel_, SLOT (setFilterFixedString (const QString&)));
    connect (Ui_.WildcardSearch_, SIGNAL (textChanged (const QString&)), ItemsFilterModel_, SLOT (setFilterWildcard (const QString&)));
    connect (Ui_.RegexpSearch_, SIGNAL (textChanged (const QString&)), ItemsFilterModel_, SLOT (setFilterRegExp (const QString&)));
    QHeaderView *itemsHeader = Ui_.Items_->header ();
    QFontMetrics fm = fontMetrics ();
    itemsHeader->resizeSection (0, fm.width ("Average news article size is about this width or maybe bigger, because they are bigger"));
    itemsHeader->resizeSection (1, fm.width ("_99 Mar 9999 99:99:99_"));

    ChannelsFilterModel_ = new ChannelsFilterModel (this);
    ChannelsFilterModel_->setSourceModel (Core::Instance ().GetChannelsModel ());
    ChannelsFilterModel_->setFilterKeyColumn (0);
    ChannelsFilterModel_->setDynamicSortFilter (true);
    Ui_.Feeds_->setModel (ChannelsFilterModel_);
    Ui_.Feeds_->addAction (Ui_.ActionMarkChannelAsRead_);
    Ui_.Feeds_->addAction (Ui_.ActionMarkChannelAsUnread_);
    Ui_.Feeds_->setContextMenuPolicy (Qt::ActionsContextMenu);
    QHeaderView *channelsHeader = Ui_.Feeds_->header ();
    channelsHeader->resizeSection (0, fm.width ("Average channel name"));
    channelsHeader->resizeSection (1, fm.width ("_99 Mar 9999 99:99:99_"));
    channelsHeader->resizeSection (2, fm.width ("_999_"));
    connect (Ui_.TagsLine_, SIGNAL (textChanged (const QString&)), ChannelsFilterModel_, SLOT (setFilterFixedString (const QString&)));
    connect (Ui_.Feeds_->selectionModel (), SIGNAL (currentChanged (const QModelIndex&, const QModelIndex&)), this, SLOT (currentChannelChanged ()));
    connect (Ui_.Items_->selectionModel (), SIGNAL (currentChanged (const QModelIndex&, const QModelIndex&)), this, SLOT (currentItemChanged (const QModelIndex&)));
    connect (Ui_.ActionUpdateFeeds_, SIGNAL (triggered ()), &Core::Instance (), SLOT (updateFeeds ()));

    TagsCompleter_ = new TagsCompleter (this);
    TagsCompleter_->setModel (Core::Instance ().GetTagsCompletionModel ());
    Core::Instance ().GetTagsCompletionModel ()->SetLineEdit (Ui_.TagsLine_);
    TagsCompleter_->setCompletionPrefix ("fuck");
    Ui_.TagsLine_->setCompleter (TagsCompleter_);
    Ui_.ChannelTags_->setCompleter (TagsCompleter_);

    Ui_.MainSplitter_->setStretchFactor (0, 5);
    Ui_.MainSplitter_->setStretchFactor (1, 9);
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

void Aggregator::handleHidePlugins ()
{
    IsShown_ = false;
    hide ();
}

void Aggregator::showError (const QString& msg)
{
    qWarning () << Q_FUNC_INFO << msg;
    QMessageBox::warning (0, tr ("Error"), msg);
}

void Aggregator::on_ActionAddFeed__triggered ()
{
    AddFeed af;
    if (af.exec () == QDialog::Accepted)
        Core::Instance ().AddFeed (af.GetURL (), af.GetTags ());
}

void Aggregator::on_ActionRemoveFeed__triggered ()
{
    Core::Instance ().RemoveFeed (ChannelsFilterModel_->mapToSource (Ui_.Feeds_->selectionModel ()->currentIndex ()));
}

void Aggregator::on_ActionPreferences__triggered ()
{
    XmlSettingsDialog_->show ();
    XmlSettingsDialog_->setWindowTitle (windowTitle () + tr (": Preferences"));
}

void Aggregator::on_Items__activated (const QModelIndex& index)
{
    Core::Instance ().Activated (ItemsFilterModel_->mapToSource (index));
}

void Aggregator::on_Feeds__activated (const QModelIndex& index)
{
    Core::Instance ().FeedActivated (ChannelsFilterModel_->mapToSource (index));
}

void Aggregator::on_ActionMarkItemAsUnread__triggered ()
{
    QModelIndexList indexes = Ui_.Items_->selectionModel ()->selectedRows ();
    for (int i = 0; i < indexes.size (); ++i)
        Core::Instance ().MarkItemAsUnread (ItemsFilterModel_->mapToSource (indexes.at (i)));
}

void Aggregator::on_ActionMarkChannelAsRead__triggered ()
{
    QModelIndexList indexes = Ui_.Feeds_->selectionModel ()->selectedRows ();
    for (int i = 0; i < indexes.size (); ++i)
        Core::Instance ().MarkChannelAsRead (ChannelsFilterModel_->mapToSource (indexes.at (i)));
}

void Aggregator::on_ActionMarkChannelAsUnread__triggered ()
{
    QModelIndexList indexes = Ui_.Feeds_->selectionModel ()->selectedRows ();
    for (int i = 0; i < indexes.size (); ++i)
        Core::Instance ().MarkChannelAsUnread (ChannelsFilterModel_->mapToSource (indexes.at (i)));
}

void Aggregator::on_ActionUpdateSelectedFeed__triggered ()
{
    QModelIndex current = Ui_.Feeds_->selectionModel ()->currentIndex ();
    if (!current.isValid ())
        return;

    Core::Instance ().UpdateFeed (ChannelsFilterModel_->mapToSource (current));
}

void Aggregator::on_ChannelTags__textChanged (const QString& tags)
{
    QModelIndex current = Ui_.Feeds_->selectionModel ()->currentIndex ();
    if (!current.isValid ())
        return;
    Core::Instance ().SetTagsForIndex (tags, ChannelsFilterModel_->mapToSource (current));
}

void Aggregator::on_ChannelTags__editingFinished ()
{
    Core::Instance ().UpdateTags (Ui_.ChannelTags_->text ().split (' '));
}

void Aggregator::on_CaseSensitiveSearch__stateChanged (int state)
{
    ItemsFilterModel_->setFilterCaseSensitivity (state ? Qt::CaseSensitive : Qt::CaseInsensitive);
}

void Aggregator::currentItemChanged (const QModelIndex& index)
{
    Ui_.ItemView_->setHtml (Core::Instance ().GetDescription (index));
}

void Aggregator::currentChannelChanged ()
{
    QModelIndex index = Ui_.Feeds_->selectionModel ()->currentIndex ();
    Core::Instance ().currentChannelChanged (ChannelsFilterModel_->mapToSource (index));
    Ui_.ChannelTags_->setText (Core::Instance ().GetTagsForIndex (ChannelsFilterModel_->mapToSource (index).row ()).join (" "));
    Ui_.ChannelLink_->setText (Core::Instance ().GetChannelLink (ChannelsFilterModel_->mapToSource (index)));
    Ui_.ChannelDescription_->setText (Core::Instance ().GetChannelDescription (ChannelsFilterModel_->mapToSource (index)));
    Ui_.ChannelAuthor_->setText (Core::Instance ().GetChannelAuthor (ChannelsFilterModel_->mapToSource (index)));
    Ui_.ChannelLanguage_->setText (Core::Instance ().GetChannelLanguage (ChannelsFilterModel_->mapToSource (index)));
}

Q_EXPORT_PLUGIN2 (leechcraft_aggregator, Aggregator);

