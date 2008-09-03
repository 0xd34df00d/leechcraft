#include <QMessageBox>
#include <QtDebug>
#include <QSortFilterProxyModel>
#include <QHeaderView>
#include <QCompleter>
#include <QSystemTrayIcon>
#include <QPainter>
#include <QMenu>
#include <QtWebKit>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <plugininterface/tagscompletionmodel.h>
#include <plugininterface/tagscompleter.h>
#include <plugininterface/util.h>
#include "aggregator.h"
#include "core.h"
#include "addfeed.h"
#include "itemsfiltermodel.h"
#include "channelsfiltermodel.h"
#include "xmlsettingsmanager.h"
#include "itembucket.h"
#include "regexpmatcherui.h"
#include "regexpmatchermanager.h"

void Aggregator::Init ()
{
	LeechCraft::Util::InstallTranslator ("aggregator");
    Ui_.setupUi (this);
    IsShown_ = false;

    TrayIcon_ = new QSystemTrayIcon (this);
    TrayIcon_->hide ();
    connect (TrayIcon_, SIGNAL (activated (QSystemTrayIcon::ActivationReason)), this, SLOT (trayIconActivated ()));

	Plugins_->addAction (Ui_.ActionAddFeed_);
	connect (&Core::Instance (),
			SIGNAL (error (const QString&)),
			this,
			SLOT (showError (const QString&)));
	connect (&Core::Instance (),
			SIGNAL (showDownloadMessage (const QString&)),
			this,
			SIGNAL (downloadFinished (const QString&)));
	connect (&Core::Instance (),
			SIGNAL (unreadNumberChanged (int)),
			this,
			SLOT (unreadNumberChanged (int)));

    XmlSettingsDialog_ = new XmlSettingsDialog (this);
    XmlSettingsDialog_->RegisterObject (XmlSettingsManager::Instance (), ":/aggregatorsettings.xml");

    Core::Instance ().DoDelayedInit ();
    ItemsFilterModel_ = new ItemsFilterModel (this);
    ItemsFilterModel_->setSourceModel (&Core::Instance ());
    ItemsFilterModel_->setFilterKeyColumn (0);
    ItemsFilterModel_->setDynamicSortFilter (true);
    ItemsFilterModel_->setFilterCaseSensitivity (Qt::CaseInsensitive);
    Ui_.Items_->setModel (ItemsFilterModel_);
    Ui_.Items_->addAction (Ui_.ActionMarkItemAsUnread_);
	Ui_.Items_->addAction (Ui_.ActionAddToItemBucket_);
    Ui_.Items_->setContextMenuPolicy (Qt::ActionsContextMenu);
    connect (Ui_.SearchLine_,
			SIGNAL (textChanged (const QString&)),
			this,
			SLOT (updateItemsFilter ()));
    connect (Ui_.SearchType_,
			SIGNAL (currentIndexChanged (int)),
			this,
			SLOT (updateItemsFilter ()));
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
    connect (Ui_.TagsLine_,
			SIGNAL (textChanged (const QString&)),
			ChannelsFilterModel_,
			SLOT (setFilterFixedString (const QString&)));
    connect (Ui_.Feeds_->selectionModel (),
			SIGNAL (currentChanged (const QModelIndex&, const QModelIndex&)),
			this,
			SLOT (currentChannelChanged ()));
    connect (Ui_.Items_->selectionModel (),
			SIGNAL (currentChanged (const QModelIndex&, const QModelIndex&)),
			this,
			SLOT (currentItemChanged (const QModelIndex&)));
    connect (Ui_.ActionUpdateFeeds_, SIGNAL (triggered ()), &Core::Instance (), SLOT (updateFeeds ()));

    TagsLineCompleter_ = new TagsCompleter (Ui_.TagsLine_, this);
    ChannelTagsCompleter_ = new TagsCompleter (Ui_.ChannelTags_, this);
    TagsLineCompleter_->setModel (Core::Instance ().GetTagsCompletionModel ());
    ChannelTagsCompleter_->setModel (Core::Instance ().GetTagsCompletionModel ());

    Ui_.MainSplitter_->setStretchFactor (0, 5);
    Ui_.MainSplitter_->setStretchFactor (1, 9);

	connect (&RegexpMatcherManager::Instance (),
			SIGNAL (gotLink (const QString&)),
			this,
			SIGNAL (fileDownloaded (const QString&)));
	connect (Ui_.MainSplitter_,
			SIGNAL (splitterMoved (int, int)),
			this,
			SLOT (updatePixmap (int)));
}

void Aggregator::Release ()
{
    Core::Instance ().Release ();
    delete XmlSettingsDialog_;
    TrayIcon_->hide ();
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
    if (!XmlSettingsManager::Instance ()->property ("Silent").toBool ())
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
	if (index.isValid ())
		Core::Instance ().Activated (ItemsFilterModel_->mapToSource (index));
}

void Aggregator::on_Feeds__activated (const QModelIndex& index)
{
	if (index.isValid ())
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

void Aggregator::on_ActionAddToItemBucket__triggered ()
{
	Core::Instance ().AddToItemBucket (ItemsFilterModel_->
			mapToSource (Ui_.Items_->selectionModel ()->
				currentIndex ()));
}

void Aggregator::on_ActionItemBucket__triggered ()
{
	ItemBucket::Instance ().show ();
}

void Aggregator::on_ActionRegexpMatcher__triggered ()
{
	RegexpMatcherUi::Instance ().show ();
}

void Aggregator::on_ActionHideReadItems__triggered ()
{
	if (Ui_.ActionHideReadItems_->isChecked ())
		Ui_.Items_->selectionModel ()->reset ();
	ItemsFilterModel_->SetHideRead (Ui_.ActionHideReadItems_->isChecked ());
}

void Aggregator::currentItemChanged (const QModelIndex& index)
{
	QModelIndex sindex = ItemsFilterModel_->mapToSource (index);
	if (!sindex.isValid ())
	{
		Ui_.ItemView_->setHtml ("");
		Ui_.ItemAuthor_->setText ("");
		Ui_.ItemCategory_->setText ("");
		Ui_.ItemLink_->setText ("");
		Ui_.ItemPubDate_->setDateTime (QDateTime ());
		return;
	}
    Ui_.ItemView_->setHtml (Core::Instance ().GetDescription (sindex));
	connect (Ui_.ItemView_->page ()->networkAccessManager (),
			SIGNAL (sslErrors (QNetworkReply*, const QList<QSslError>&)),
			&Core::Instance (),
			SLOT (handleSslError (QNetworkReply*)));
	Ui_.ItemAuthor_->setText (Core::Instance ().GetAuthor (sindex));
	Ui_.ItemCategory_->setText (Core::Instance ().GetCategory (sindex));
	QString link = Core::Instance ().GetLink (sindex);
	QString shortLink;
    if (link.size () >= 40)
        shortLink = link.left (15) + "..." + link.right (15);
    else
        shortLink = link;
	if(QUrl (link).isValid ())
	{
	    link.insert (0,"<a href=\"");
	    link.append ("\">" + shortLink + "</a>");
        Ui_.ItemLink_->setText (link);
        Ui_.ItemLink_->setOpenExternalLinks (true);
	}
	else
	{
        Ui_.ItemLink_->setOpenExternalLinks (false);
        Ui_.ItemLink_->setText (shortLink);
    }
	Ui_.ItemPubDate_->setDateTime (Core::Instance ().GetPubDate (sindex));
}

void Aggregator::currentChannelChanged ()
{
    QModelIndex index = Ui_.Feeds_->selectionModel ()->currentIndex ();
	if (!index.isValid ())
		return;

	QModelIndex mapped = ChannelsFilterModel_->mapToSource (index);
    Core::Instance ().currentChannelChanged (mapped);
    Ui_.ChannelTags_->setText (Core::Instance ().GetTagsForIndex (mapped.row ()).join (" "));
	QString link = Core::Instance ().GetChannelLink (mapped);
	if (link.size () >= 40)
		link = link.left (15) + "..." + link.right (15);
    Ui_.ChannelLink_->setText (link);
	Ui_.ChannelDescription_->setText (Core::Instance ().GetChannelDescription (mapped));
    Ui_.ChannelAuthor_->setText (Core::Instance ().GetChannelAuthor (mapped));
    Ui_.ChannelLanguage_->setText (Core::Instance ().GetChannelLanguage (mapped));
	Ui_.ItemView_->setHtml ("");

	updatePixmap (Ui_.MainSplitter_->sizes ().at (0));
}

void Aggregator::unreadNumberChanged (int number)
{
    if (!number || !XmlSettingsManager::Instance ()->property ("ShowIconInTray").toBool ())
    {
        TrayIcon_->hide ();
        return;
    }

    QIcon icon (":/resources/images/trayicon.png");
    QPixmap pixmap = icon.pixmap (22, 22);
    QPainter painter;
    painter.begin (&pixmap);
    QFont font = QApplication::font ();
    font.setBold (true);
    font.setPointSize (14);
    font.setFamily ("Arial");
    painter.setFont (font);
    painter.setPen (Qt::blue);
    painter.setRenderHints (QPainter::TextAntialiasing);
    painter.drawText (0, 0, 21, 21, Qt::AlignBottom | Qt::AlignRight, QString::number (number));
    painter.end ();

    TrayIcon_->setIcon (QIcon (pixmap));
    TrayIcon_->show ();
}

void Aggregator::trayIconActivated ()
{
    show ();
    IsShown_ = true;
	QModelIndex unread = Core::Instance ().GetUnreadChannelIndex ();
	if (unread.isValid ())
		Ui_.Feeds_->setCurrentIndex (ChannelsFilterModel_->mapFromSource (unread));
}

void Aggregator::updateItemsFilter ()
{
	int section = Ui_.SearchType_->currentIndex ();
	QString text = Ui_.SearchLine_->text ();
	switch (section)
	{
	case 1:
		ItemsFilterModel_->setFilterWildcard (text);
		break;
	case 2:
		ItemsFilterModel_->setFilterRegExp (text);
		break;
	default:
		ItemsFilterModel_->setFilterFixedString (text);
		break;
	}
}

void Aggregator::updatePixmap (int width)
{
    QModelIndex index = Ui_.Feeds_->selectionModel ()->currentIndex ();
	if (!index.isValid ())
		return;

	QModelIndex mapped = ChannelsFilterModel_->mapToSource (index);

	QPixmap pixmap = Core::Instance ().GetChannelPixmap (mapped);
	if (!pixmap.isNull ())
		Ui_.ChannelImage_->setPixmap (pixmap.scaledToWidth (width,
					Qt::SmoothTransformation));
}

Q_EXPORT_PLUGIN2 (leechcraft_aggregator, Aggregator);

