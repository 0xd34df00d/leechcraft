#include <QMessageBox>
#include <QtDebug>
#include <QSortFilterProxyModel>
#include <QHeaderView>
#include <QCompleter>
#include <QSystemTrayIcon>
#include <QPainter>
#include <QMenu>
#include <QToolBar>
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
#include "importopml.h"
#include "exportopml.h"

Aggregator::~Aggregator ()
{
}

void Aggregator::Init ()
{
	Translator_.reset (LeechCraft::Util::InstallTranslator ("aggregator"));
	SetupMenuBar ();
    Ui_.setupUi (this);
	dynamic_cast<QVBoxLayout*> (layout ())->insertWidget (0, ToolBar_);

    TrayIcon_.reset (new QSystemTrayIcon (this));
    TrayIcon_->hide ();
    connect (TrayIcon_.get (),
			SIGNAL (activated (QSystemTrayIcon::ActivationReason)),
			this,
			SLOT (trayIconActivated ()));

	Plugins_->addAction (ActionAddFeed_);
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

    XmlSettingsDialog_.reset (new XmlSettingsDialog (this));
    XmlSettingsDialog_->RegisterObject (XmlSettingsManager::Instance (), ":/aggregatorsettings.xml");

    Core::Instance ().DoDelayedInit ();
    ItemsFilterModel_.reset (new ItemsFilterModel (this));
    ItemsFilterModel_->setSourceModel (&Core::Instance ());
    ItemsFilterModel_->setFilterKeyColumn (0);
    ItemsFilterModel_->setDynamicSortFilter (true);
    ItemsFilterModel_->setFilterCaseSensitivity (Qt::CaseInsensitive);
    Ui_.Items_->setModel (ItemsFilterModel_.get ());
    Ui_.Items_->addAction (ActionMarkItemAsUnread_);
	Ui_.Items_->addAction (ActionAddToItemBucket_);
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
	int dateTimeSize = fm.width (QDateTime::currentDateTime ().toString (Qt::SystemLocaleShortDate)) + fm.width("__");
	itemsHeader->resizeSection (0,
			fm.width ("Average news article size is about this width or maybe bigger, because they are bigger"));
	itemsHeader->resizeSection (1,
			dateTimeSize);

    ChannelsFilterModel_.reset (new ChannelsFilterModel (this));
    ChannelsFilterModel_->setSourceModel (Core::Instance ().GetChannelsModel ());
    ChannelsFilterModel_->setFilterKeyColumn (0);
    ChannelsFilterModel_->setDynamicSortFilter (true);
    Ui_.Feeds_->setModel (ChannelsFilterModel_.get ());
    Ui_.Feeds_->addAction (ActionMarkChannelAsRead_);
    Ui_.Feeds_->addAction (ActionMarkChannelAsUnread_);
    Ui_.Feeds_->setContextMenuPolicy (Qt::ActionsContextMenu);
    QHeaderView *channelsHeader = Ui_.Feeds_->header ();
    channelsHeader->resizeSection (0, fm.width ("Average channel name"));
    channelsHeader->resizeSection (1, dateTimeSize);
    channelsHeader->resizeSection (2, fm.width ("_999_"));
    connect (Ui_.TagsLine_,
			SIGNAL (textChanged (const QString&)),
			ChannelsFilterModel_.get (),
			SLOT (setFilterFixedString (const QString&)));
    connect (Ui_.Feeds_->selectionModel (),
			SIGNAL (currentChanged (const QModelIndex&, const QModelIndex&)),
			this,
			SLOT (currentChannelChanged ()));
    connect (Ui_.Items_->selectionModel (),
			SIGNAL (currentChanged (const QModelIndex&, const QModelIndex&)),
			this,
			SLOT (currentItemChanged (const QModelIndex&)));
    connect (ActionUpdateFeeds_,
			SIGNAL (triggered ()),
			&Core::Instance (),
			SLOT (updateFeeds ()));

    TagsLineCompleter_.reset (new TagsCompleter (Ui_.TagsLine_));
    ChannelTagsCompleter_.reset (new TagsCompleter (Ui_.ChannelTags_));
    TagsLineCompleter_->setModel (Core::Instance ().GetTagsCompletionModel ());
    ChannelTagsCompleter_->setModel (Core::Instance ().GetTagsCompletionModel ());

	Ui_.ChannelSplitter_->setStretchFactor (0, 5);
	Ui_.ChannelSplitter_->setStretchFactor (1, 2);
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

	XmlSettingsManager::Instance ()->RegisterObject ("StandardFont", this, "viewerSettingsChanged");
	XmlSettingsManager::Instance ()->RegisterObject ("FixedFont", this, "viewerSettingsChanged");
	XmlSettingsManager::Instance ()->RegisterObject ("SerifFont", this, "viewerSettingsChanged");
	XmlSettingsManager::Instance ()->RegisterObject ("SansSerifFont", this, "viewerSettingsChanged");
	XmlSettingsManager::Instance ()->RegisterObject ("CursiveFont", this, "viewerSettingsChanged");
	XmlSettingsManager::Instance ()->RegisterObject ("FantasyFont", this, "viewerSettingsChanged");
	XmlSettingsManager::Instance ()->RegisterObject ("MinimumFontSize", this, "viewerSettingsChanged");
	XmlSettingsManager::Instance ()->RegisterObject ("DefaultFontSize", this, "viewerSettingsChanged");
	XmlSettingsManager::Instance ()->RegisterObject ("DefaultFixedFontSize", this, "viewerSettingsChanged");
	XmlSettingsManager::Instance ()->RegisterObject ("AutoLoadImages", this, "viewerSettingsChanged");
	XmlSettingsManager::Instance ()->RegisterObject ("AllowJavaScript", this, "viewerSettingsChanged");

	viewerSettingsChanged ();
}

void Aggregator::Release ()
{
    Core::Instance ().Release ();
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

QWidget* Aggregator::GetTabContents ()
{
	return this;
}

void Aggregator::SetupMenuBar ()
{
	ToolBar_ = new QToolBar (this);

	ActionAddFeed_ = new QAction (QIcon (":/resources/images/rss-feed.png"),
			tr ("Add feed..."),
			this);
	ActionAddFeed_->setObjectName ("ActionAddFeed_");

	ActionPreferences_ = new QAction (QIcon (":/resources/images/preferences.png"),
			tr ("Preferences..."),
			this);
	ActionPreferences_->setObjectName ("ActionPreferences_");

	ActionUpdateFeeds_ = new QAction (QIcon (":/resources/images/updateallfeeds.png"),
			tr ("Update all feeds"),
			this);

	ActionRemoveFeed_ = new QAction (QIcon (":/resources/images/removefeed.png"),
			tr ("Remove feed"),
			this);
	ActionRemoveFeed_->setObjectName ("ActionRemoveFeed_");

	ActionMarkItemAsUnread_ = new QAction (tr ("Mark item as unread"),
			this);
	ActionMarkItemAsUnread_->setObjectName ("ActionMarkItemAsUnread_");

	ActionMarkChannelAsRead_ = new QAction (tr ("Mark channel as read"),
			this);
	ActionMarkChannelAsRead_->setObjectName ("ActionMarkChannelAsRead_");

	ActionMarkChannelAsUnread_ = new QAction (tr ("Mark channel as unread"),
			this);
	ActionMarkChannelAsUnread_->setObjectName ("ActionMarkChannelAsUnread_");

	ActionUpdateSelectedFeed_ = new QAction (QIcon (":/resources/images/updateselectedfeed.png"),
			tr ("Update selected feed"),
			this);
	ActionUpdateSelectedFeed_->setObjectName ("ActionUpdateSelectedFeed_");

	ActionAddToItemBucket_ = new QAction (tr ("Add to item bucket"),
			this);
	ActionAddToItemBucket_->setObjectName ("ActionAddToItemBucket_");

	ActionItemBucket_ = new QAction (QIcon (":/resources/images/favorites.png"),
			tr ("Item bucket..."),
			this);
	ActionItemBucket_->setObjectName ("ActionItemBucket_");

	ActionRegexpMatcher_ = new QAction (QIcon (":/resources/images/filter.png"),
			tr ("Regexp matcher..."),
			this);
	ActionRegexpMatcher_->setObjectName ("ActionRegexpMatcher_");

	ActionHideReadItems_ = new QAction (tr ("Hide read items"),
			this);
	ActionHideReadItems_->setObjectName ("ActionHideReadItems_");
	ActionHideReadItems_->setCheckable (true);
	QIcon icon;
	icon.addPixmap (QPixmap (":/resources/images/rss-show.png"), QIcon::Normal, QIcon::Off);
	icon.addPixmap (QPixmap (":/resources/images/rss-hide.png"), QIcon::Normal, QIcon::On);
	ActionHideReadItems_->setIcon (icon);

	ActionImportOPML_ = new QAction (tr ("Import OPML..."),
			this);
	ActionImportOPML_->setObjectName ("ActionImportOPML_");

	ActionExportOPML_ = new QAction (tr ("Export OPML..."),
			this);
	ActionExportOPML_->setObjectName ("ActionExportOPML_");

    ToolBar_->addAction(ActionAddFeed_);
    ToolBar_->addAction(ActionRemoveFeed_);
    ToolBar_->addAction(ActionUpdateSelectedFeed_);
    ToolBar_->addAction(ActionUpdateFeeds_);
    ToolBar_->addSeparator();
    ToolBar_->addAction(ActionItemBucket_);
    ToolBar_->addAction(ActionRegexpMatcher_);
    ToolBar_->addSeparator();
    ToolBar_->addAction(ActionImportOPML_);
    ToolBar_->addAction(ActionExportOPML_);
    ToolBar_->addSeparator();
    ToolBar_->addAction(ActionHideReadItems_);
    ToolBar_->addSeparator();
    ToolBar_->addAction(ActionPreferences_);
}

void Aggregator::showError (const QString& msg)
{
    qWarning () << Q_FUNC_INFO << msg;
    if (!XmlSettingsManager::Instance ()->property ("BeSilent").toBool ())
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
	QMessageBox mb (QMessageBox::Warning,
			tr ("Warning"),
			tr ("You are going to permanently remove this feed. "
				"Are you are really sure that you want to do this?", "Feed removing confirmation"),
			QMessageBox::Ok | QMessageBox::Cancel,
			this);
	mb.setWindowModality (Qt::WindowModal);
	if (mb.exec () == QMessageBox::Ok)
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
	if (ActionHideReadItems_->isChecked ())
		Ui_.Items_->selectionModel ()->reset ();
	ItemsFilterModel_->SetHideRead (ActionHideReadItems_->isChecked ());
}

void Aggregator::on_ActionImportOPML__triggered ()
{
	ImportOPML importDialog;
	if (importDialog.exec () == QDialog::Rejected)
		return;

	Core::Instance ().AddFromOPML (importDialog.GetFilename (),
			importDialog.GetTags (),
			importDialog.GetMask ());
}

void Aggregator::on_ActionExportOPML__triggered ()
{
	ExportOPML exportDialog;
	exportDialog.SetFeeds (Core::Instance ().GetFeeds ());
	if (exportDialog.exec () == QDialog::Rejected)
		return;

	Core::Instance ().ExportToOPML (exportDialog.GetDestination (),
			exportDialog.GetTitle (),
			exportDialog.GetOwner (),
			exportDialog.GetOwnerEmail (),
			exportDialog.GetSelectedFeeds ());
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
	Ui_.ItemLink_->setToolTip (link);
	if (link.size () >= 40)
		shortLink = link.left (18) + "..." + link.right (18);
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
	Core::ChannelInfo ci = Core::Instance ().GetChannelInfo (mapped);
	QString link = ci.Link_;
	QString shortLink;
	Ui_.ChannelLink_->setToolTip (link);
	if (link.size () >= 40)
		shortLink = link.left (18) + "..." + link.right (18);
	else
		shortLink = link;
	if (QUrl (link).isValid ())
	{
		link.insert (0,"<a href=\"");
		link.append ("\">" + shortLink + "</a>");
		Ui_.ChannelLink_->setText (link);
		Ui_.ChannelLink_->setOpenExternalLinks (true);
	}
	else
	{
		Ui_.ChannelLink_->setOpenExternalLinks (false);
		Ui_.ChannelLink_->setText (shortLink);
	}
	Ui_.ChannelDescription_->setHtml (ci.Description_);
	Ui_.ChannelAuthor_->setText (ci.Author_);
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
	emit bringToFront ();
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
	Ui_.ChannelImage_->setPixmap (pixmap.scaledToWidth (width,
				Qt::SmoothTransformation));
}

void Aggregator::viewerSettingsChanged ()
{
	Ui_.ItemView_->settings ()->setFontFamily (QWebSettings::StandardFont,
			XmlSettingsManager::Instance ()->property ("StandardFont").value<QFont> ().family ());
	Ui_.ItemView_->settings ()->setFontFamily (QWebSettings::FixedFont,
			XmlSettingsManager::Instance ()->property ("FixedFont").value<QFont> ().family ());
	Ui_.ItemView_->settings ()->setFontFamily (QWebSettings::SerifFont,
			XmlSettingsManager::Instance ()->property ("SerifFont").value<QFont> ().family ());
	Ui_.ItemView_->settings ()->setFontFamily (QWebSettings::SansSerifFont,
			XmlSettingsManager::Instance ()->property ("SansSerifFont").value<QFont> ().family ());
	Ui_.ItemView_->settings ()->setFontFamily (QWebSettings::CursiveFont,
			XmlSettingsManager::Instance ()->property ("CursiveFont").value<QFont> ().family ());
	Ui_.ItemView_->settings ()->setFontFamily (QWebSettings::FantasyFont,
			XmlSettingsManager::Instance ()->property ("FantasyFont").value<QFont> ().family ());

	Ui_.ItemView_->settings ()->setFontSize (QWebSettings::MinimumFontSize,
			XmlSettingsManager::Instance ()->property ("MinimumFontSize").toInt ());
	Ui_.ItemView_->settings ()->setFontSize (QWebSettings::DefaultFontSize,
			XmlSettingsManager::Instance ()->property ("DefaultFontSize").toInt ());
	Ui_.ItemView_->settings ()->setFontSize (QWebSettings::DefaultFixedFontSize,
			XmlSettingsManager::Instance ()->property ("DefaultFixedFontSize").toInt ());
	Ui_.ItemView_->settings ()->setAttribute (QWebSettings::AutoLoadImages,
			XmlSettingsManager::Instance ()->property ("AutoLoadImages").toBool ());
	Ui_.ItemView_->settings ()->setAttribute (QWebSettings::JavascriptEnabled,
			XmlSettingsManager::Instance ()->property ("AllowJavaScript").toBool ());
}

Q_EXPORT_PLUGIN2 (leechcraft_aggregator, Aggregator);

