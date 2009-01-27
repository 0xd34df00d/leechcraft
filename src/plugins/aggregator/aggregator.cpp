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
#include <QCursor>
#include <QKeyEvent>
#include <plugininterface/tagscompletionmodel.h>
#include <plugininterface/util.h>
#include <plugininterface/proxy.h>
#include <plugininterface/categoryselector.h>
#include <plugininterface/tagscompleter.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "ui_mainwidget.h"
#include "itemsfiltermodel.h"
#include "channelsfiltermodel.h"
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
#include "export.h"
#include "itembucket.h"
#include "importbinary.h"
#include "feedsettings.h"

using LeechCraft::Util::TagsCompleter;
using LeechCraft::Util::CategorySelector;

struct Aggregator_Impl
{
    Ui::MainWidget Ui_;

	QToolBar *ToolBar_;
    QAction *ActionAddFeed_;
    QAction *ActionUpdateFeeds_;
    QAction *ActionRemoveFeed_;
    QAction *ActionMarkItemAsUnread_;
    QAction *ActionMarkChannelAsRead_;
    QAction *ActionMarkChannelAsUnread_;
	QAction *ActionChannelSettings_;
    QAction *ActionUpdateSelectedFeed_;
    QAction *ActionAddToItemBucket_;
    QAction *ActionItemBucket_;
    QAction *ActionRegexpMatcher_;
    QAction *ActionHideReadItems_;
    QAction *ActionImportOPML_;
    QAction *ActionExportOPML_;
	QAction *ActionImportBinary_;
	QAction *ActionExportBinary_;

	std::auto_ptr<LeechCraft::Util::XmlSettingsDialog> XmlSettingsDialog_;
	std::auto_ptr<ItemsFilterModel> ItemsFilterModel_;
	std::auto_ptr<ChannelsFilterModel> ChannelsFilterModel_;
	std::auto_ptr<LeechCraft::Util::TagsCompleter> TagsLineCompleter_;
	std::auto_ptr<QSystemTrayIcon> TrayIcon_;
	std::auto_ptr<QTranslator> Translator_;
    std::auto_ptr<ItemBucket> ItemBucket_;
	std::auto_ptr<LeechCraft::Util::CategorySelector> ItemCategorySelector_;
	std::auto_ptr<RegexpMatcherUi> RegexpMatcherUi_;
};

Aggregator::~Aggregator ()
{
}

void Aggregator::Init ()
{
	Impl_ = new Aggregator_Impl;
	Impl_->Translator_.reset (LeechCraft::Util::InstallTranslator ("aggregator"));
	SetupMenuBar ();
	Impl_->Ui_.setupUi (this);

	Impl_->RegexpMatcherUi_.reset (new RegexpMatcherUi (this));

	Impl_->ItemBucket_.reset (new ItemBucket (this));
	dynamic_cast<QVBoxLayout*> (layout ())->insertWidget (0, Impl_->ToolBar_);

	Impl_->TrayIcon_.reset (new QSystemTrayIcon (this));
	Impl_->TrayIcon_->hide ();
	connect (Impl_->TrayIcon_.get (),
			SIGNAL (activated (QSystemTrayIcon::ActivationReason)),
			this,
			SLOT (trayIconActivated ()));

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

	Impl_->XmlSettingsDialog_.reset (new LeechCraft::Util::XmlSettingsDialog ());
	Impl_->XmlSettingsDialog_->RegisterObject (XmlSettingsManager::Instance (), ":/aggregatorsettings.xml");

	Core::Instance ().DoDelayedInit ();
	Impl_->ItemsFilterModel_.reset (new ItemsFilterModel (this));
	Impl_->ItemsFilterModel_->setSourceModel (&Core::Instance ());
	connect (&Core::Instance (),
			SIGNAL (dataChanged (const QModelIndex&, const QModelIndex&)),
			Impl_->ItemsFilterModel_.get (),
			SLOT (invalidate ()));
	Impl_->ItemsFilterModel_->setFilterKeyColumn (0);
	Impl_->ItemsFilterModel_->setDynamicSortFilter (true);
	Impl_->ItemsFilterModel_->setFilterCaseSensitivity (Qt::CaseInsensitive);
	Impl_->Ui_.Items_->setModel (Impl_->ItemsFilterModel_.get ());

	Impl_->ItemCategorySelector_.reset (new CategorySelector ());
	connect (Impl_->ItemCategorySelector_.get (),
			SIGNAL (selectionChanged (const QStringList&)),
			Impl_->ItemsFilterModel_.get (),
			SLOT (categorySelectionChanged (const QStringList&)));

	Impl_->Ui_.Items_->addAction (Impl_->ActionMarkItemAsUnread_);
	Impl_->Ui_.Items_->addAction (Impl_->ActionAddToItemBucket_);
	Impl_->Ui_.Items_->setContextMenuPolicy (Qt::ActionsContextMenu);
	connect (Impl_->Ui_.SearchLine_,
			SIGNAL (textChanged (const QString&)),
			this,
			SLOT (updateItemsFilter ()));
	connect (Impl_->Ui_.SearchType_,
			SIGNAL (currentIndexChanged (int)),
			this,
			SLOT (updateItemsFilter ()));
	QHeaderView *itemsHeader = Impl_->Ui_.Items_->header ();
	QFontMetrics fm = fontMetrics ();
	int dateTimeSize = fm.width (QDateTime::currentDateTime ()
			.toString (Qt::SystemLocaleShortDate) + "__");
	itemsHeader->resizeSection (0,
			fm.width ("Average news article size is about this width or "
				"maybe bigger, because they are bigger"));
	itemsHeader->resizeSection (1,
			dateTimeSize);
	connect (Impl_->Ui_.Items_->header (),
			SIGNAL (sectionClicked (int)),
			this,
			SLOT (makeCurrentItemVisible ()));
	Impl_->ChannelsFilterModel_.reset (new ChannelsFilterModel (this));
	Impl_->ChannelsFilterModel_->setSourceModel (Core::Instance ().GetChannelsModel ());
	Impl_->ChannelsFilterModel_->setFilterKeyColumn (0);
	Impl_->ChannelsFilterModel_->setDynamicSortFilter (true);
	Impl_->Ui_.Feeds_->setModel (Impl_->ChannelsFilterModel_.get ());
	Impl_->Ui_.Feeds_->addAction (Impl_->ActionMarkChannelAsRead_);
	Impl_->Ui_.Feeds_->addAction (Impl_->ActionMarkChannelAsUnread_);
	QAction *sep = new QAction (0);
	sep->setSeparator (true);
	Impl_->Ui_.Feeds_->addAction (sep);
	Impl_->Ui_.Feeds_->addAction (Impl_->ActionChannelSettings_);
	Impl_->Ui_.Feeds_->setContextMenuPolicy (Qt::ActionsContextMenu);
	QHeaderView *channelsHeader = Impl_->Ui_.Feeds_->header ();
	channelsHeader->resizeSection (0, fm.width ("Average channel name"));
	channelsHeader->resizeSection (1, dateTimeSize);
	channelsHeader->resizeSection (2, fm.width ("_999_"));
	connect (Impl_->Ui_.TagsLine_,
			SIGNAL (textChanged (const QString&)),
			Impl_->ChannelsFilterModel_.get (),
			SLOT (setFilterFixedString (const QString&)));
	connect (Impl_->Ui_.Feeds_->selectionModel (),
			SIGNAL (currentChanged (const QModelIndex&, const QModelIndex&)),
			this,
			SLOT (currentChannelChanged ()));
	connect (Impl_->Ui_.Items_->selectionModel (),
			SIGNAL (selectionChanged (const QItemSelection&, const QItemSelection&)),
			this,
			SLOT (currentItemChanged (const QItemSelection&)));
	connect (Impl_->ActionUpdateFeeds_,
			SIGNAL (triggered ()),
			&Core::Instance (),
			SLOT (updateFeeds ()));

	Impl_->TagsLineCompleter_.reset (new TagsCompleter (Impl_->Ui_.TagsLine_));
	Impl_->TagsLineCompleter_->setModel (Core::Instance ().GetTagsCompletionModel ());
	Impl_->Ui_.TagsLine_->AddSelector ();

	Impl_->Ui_.MainSplitter_->setStretchFactor (0, 5);
	Impl_->Ui_.MainSplitter_->setStretchFactor (1, 9);

	connect (Impl_->Ui_.ItemLink_,
			SIGNAL (linkActivated (const QString&)),
			&Core::Instance (),
			SLOT (openLink (const QString&)));

	connect (&RegexpMatcherManager::Instance (),
			SIGNAL (gotLink (const QByteArray&)),
			this,
			SIGNAL (gotEntity (const QByteArray&)));

	QList<QByteArray> viewerSettings;
	viewerSettings << "StandardFont"
		<< "FixedFont"
		<< "SerifFont"
		<< "SansSerifFont"
		<< "CursiveFont"
		<< "FantasyFont"
		<< "MinimumFontSize"
		<< "DefaultFontSize"
		<< "DefaultFixedFontSize"
		<< "AutoLoadImages"
		<< "AllowJavaScript";
	XmlSettingsManager::Instance ()->RegisterObject (viewerSettings,
			this, "viewerSettingsChanged");

	viewerSettingsChanged ();
	currentChannelChanged ();
}

void Aggregator::Release ()
{
	disconnect (&Core::Instance (), 0, this, 0);
	disconnect (Impl_->ItemsFilterModel_.get (), 0, this, 0);
	disconnect (Impl_->ChannelsFilterModel_.get (), 0, this, 0);
	disconnect (Impl_->TagsLineCompleter_.get (), 0, this, 0);
	disconnect (Impl_->ItemCategorySelector_.get (), 0, this, 0);
    Impl_->TrayIcon_->hide ();
	delete Impl_;
    Core::Instance ().Release ();
}

QString Aggregator::GetName () const
{
    return "Aggregator";
}

QString Aggregator::GetInfo () const
{
    return tr ("RSS/Atom feed reader.");
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
    return QStringList ("webbrowser");
}

void Aggregator::SetProvider (QObject* object, const QString& feature)
{
    Core::Instance ().SetProvider (object, feature);
}

QIcon Aggregator::GetIcon () const
{
    return windowIcon ();
}

QWidget* Aggregator::GetTabContents ()
{
	return this;
}

LeechCraft::Util::XmlSettingsDialog* Aggregator::GetSettingsDialog () const
{
	return Impl_->XmlSettingsDialog_.get ();
}

void Aggregator::keyPressEvent (QKeyEvent *e)
{
	if (e->modifiers () & Qt::ControlModifier)
	{
		QItemSelectionModel *channelSM = Impl_->Ui_.Feeds_->selectionModel ();
		QModelIndex currentChannel = channelSM->currentIndex ();
		int numChannels = Impl_->Ui_.Feeds_->model ()->rowCount ();

		QItemSelectionModel::SelectionFlags chanSF =
			QItemSelectionModel::Select |
			QItemSelectionModel::Clear |
			QItemSelectionModel::Rows;
		if (e->key () == Qt::Key_Less &&
				currentChannel.isValid () &&
				numChannels)
		{
			if (currentChannel.row () > 0)
			{
				QModelIndex next = currentChannel
					.sibling (currentChannel.row () - 1,
								currentChannel.column ());
				channelSM->select (next, chanSF);
				channelSM->setCurrentIndex (next, chanSF);
			}
			else
			{
				QModelIndex next = currentChannel.sibling (numChannels - 1,
								currentChannel.column ());
				channelSM->select (next, chanSF);
				channelSM->setCurrentIndex (next, chanSF);
			}
			return;
		}
		else if (e->key () == Qt::Key_Greater &&
				currentChannel.isValid () &&
				numChannels)
		{
			if (currentChannel.row () < numChannels - 1)
			{
				QModelIndex next = currentChannel
					.sibling (currentChannel.row () + 1,
								currentChannel.column ());
				channelSM->select (next, chanSF);
				channelSM->setCurrentIndex (next, chanSF);
			}
			else
			{
				QModelIndex next = currentChannel.sibling (0,
								currentChannel.column ());
				channelSM->select (next, chanSF);
				channelSM->setCurrentIndex (next, chanSF);
			}
			return;
		}
		else if ((e->key () == Qt::Key_Greater ||
				e->key () == Qt::Key_Less) &&
				numChannels &&
				!currentChannel.isValid ())
		{
			QModelIndex next = Impl_->Ui_.Feeds_->model ()->index (0, 0);
			channelSM->select (next, chanSF);
			channelSM->setCurrentIndex (next, chanSF);
		}
	}
	e->ignore ();
}

void Aggregator::SetupMenuBar ()
{
	Impl_->ToolBar_ = new QToolBar (this);

	Impl_->ActionAddFeed_ = new QAction (tr ("Add feed..."),
			this);
	Impl_->ActionAddFeed_->setObjectName ("ActionAddFeed_");
	Impl_->ActionAddFeed_->setProperty ("ActionIcon", "aggregator_add");

	Impl_->ActionUpdateFeeds_ = new QAction (tr ("Update all feeds"),
			this);
	Impl_->ActionUpdateFeeds_->setProperty ("ActionIcon", "aggregator_updateallfeeds");

	Impl_->ActionRemoveFeed_ = new QAction (tr ("Remove feed"),
			this);
	Impl_->ActionRemoveFeed_->setObjectName ("ActionRemoveFeed_");
	Impl_->ActionRemoveFeed_->setProperty ("ActionIcon", "aggregator_remove");

	Impl_->ActionMarkItemAsUnread_ = new QAction (tr ("Mark item as unread"),
			this);
	Impl_->ActionMarkItemAsUnread_->setObjectName ("ActionMarkItemAsUnread_");

	Impl_->ActionMarkChannelAsRead_ = new QAction (tr ("Mark channel as read"),
			this);
	Impl_->ActionMarkChannelAsRead_->setObjectName ("ActionMarkChannelAsRead_");

	Impl_->ActionMarkChannelAsUnread_ = new QAction (tr ("Mark channel as unread"),
			this);
	Impl_->ActionMarkChannelAsUnread_->setObjectName ("ActionMarkChannelAsUnread_");

	Impl_->ActionChannelSettings_ = new QAction (tr ("Settings..."),
			this);
	Impl_->ActionChannelSettings_->setObjectName ("ActionChannelSettings_");

	Impl_->ActionUpdateSelectedFeed_ = new QAction (tr ("Update selected feed"),
			this);
	Impl_->ActionUpdateSelectedFeed_->setObjectName ("ActionUpdateSelectedFeed_");
	Impl_->ActionUpdateSelectedFeed_->setProperty ("ActionIcon", "aggregator_updateselectedfeed");

	Impl_->ActionAddToItemBucket_ = new QAction (tr ("Add to item bucket"),
			this);
	Impl_->ActionAddToItemBucket_->setObjectName ("ActionAddToItemBucket_");

	Impl_->ActionItemBucket_ = new QAction (tr ("Item bucket..."),
			this);
	Impl_->ActionItemBucket_->setObjectName ("ActionItemBucket_");
	Impl_->ActionItemBucket_->setProperty ("ActionIcon", "aggregator_favorites");

	Impl_->ActionRegexpMatcher_ = new QAction (tr ("Regexp matcher..."),
			this);
	Impl_->ActionRegexpMatcher_->setObjectName ("ActionRegexpMatcher_");
	Impl_->ActionRegexpMatcher_->setProperty ("ActionIcon", "aggregator_filter");

	Impl_->ActionHideReadItems_ = new QAction (tr ("Hide read items"),
			this);
	Impl_->ActionHideReadItems_->setObjectName ("ActionHideReadItems_");
	Impl_->ActionHideReadItems_->setCheckable (true);
	Impl_->ActionHideReadItems_->setProperty ("ActionIcon", "aggregator_rssshow");
	Impl_->ActionHideReadItems_->setProperty ("ActionIconOff", "aggregator_rsshide");

	Impl_->ActionImportOPML_ = new QAction (tr ("Import from OPML..."),
			this);
	Impl_->ActionImportOPML_->setObjectName ("ActionImportOPML_");
	Impl_->ActionImportOPML_->setProperty ("ActionIcon", "aggregator_importopml");

	Impl_->ActionExportOPML_ = new QAction (tr ("Export to OPML..."),
			this);
	Impl_->ActionExportOPML_->setObjectName ("ActionExportOPML_");
	Impl_->ActionExportOPML_->setProperty ("ActionIcon", "aggregator_exportopml");

	Impl_->ActionImportBinary_ = new QAction (tr ("Import from binary..."),
			this);
	Impl_->ActionImportBinary_->setObjectName ("ActionImportBinary_");
	Impl_->ActionImportBinary_->setProperty ("ActionIcon", "aggregator_importbinary");

	Impl_->ActionExportBinary_ = new QAction (tr ("Export to binary..."),
			this);
	Impl_->ActionExportBinary_->setObjectName ("ActionExportBinary_");
	Impl_->ActionExportBinary_->setProperty ("ActionIcon", "aggregator_exportbinary");

	Impl_->ToolBar_->addAction (Impl_->ActionAddFeed_);
	Impl_->ToolBar_->addAction (Impl_->ActionRemoveFeed_);
	Impl_->ToolBar_->addAction (Impl_->ActionUpdateSelectedFeed_);
	Impl_->ToolBar_->addAction (Impl_->ActionUpdateFeeds_);
	Impl_->ToolBar_->addSeparator ();
	Impl_->ToolBar_->addAction (Impl_->ActionItemBucket_);
	Impl_->ToolBar_->addAction (Impl_->ActionRegexpMatcher_);
	Impl_->ToolBar_->addSeparator ();
	Impl_->ToolBar_->addAction (Impl_->ActionImportOPML_);
	Impl_->ToolBar_->addAction (Impl_->ActionExportOPML_);
	Impl_->ToolBar_->addAction (Impl_->ActionImportBinary_);
	Impl_->ToolBar_->addAction (Impl_->ActionExportBinary_);
	Impl_->ToolBar_->addSeparator ();
	Impl_->ToolBar_->addAction (Impl_->ActionHideReadItems_);
}

void Aggregator::SetHtml (const QString& title,
		const QString& description,
		const QList<Enclosure>& enclosures)
{
	QString result = "<div style='background: lightgray; "
		"border: 1px solid #000000; "
		"padding-left: 2em; "
		"padding-right: 2em;'>";
	result += title;
	result += "</div>";
	result += description;
	for (QList<Enclosure>::const_iterator i = enclosures.begin (),
			end = enclosures.end (); i != end; ++i)
	{
		result += "<div style='background: lightgray; "
			"border: 1px solid #333333; "
			"padding-top: 1em; "
			"padding-bottom: 1em; "
			"padding-left: 2em; "
			"padding-right: 2em;'>";
		if (i->Length_ > 0)
			result += tr ("File of type %1, size %2:<br />")
				.arg (i->Type_)
				.arg (LeechCraft::Util::Proxy::Instance ()->
						MakePrettySize (i->Length_));
		else
			result += tr ("File of type %1 and unknown length:<br />")
				.arg (i->Type_);
		result += QString ("<a href='%1'>%2</a>")
			.arg (i->URL_)
			.arg (QFileInfo (QUrl (i->URL_).path ()).fileName ());
		if (!i->Lang_.isEmpty ())
			result += tr ("<br />Specified language: %1")
				.arg (i->Lang_);
		result += "</div>";
	}
	Impl_->Ui_.ItemView_->setHtml (result);
}

void Aggregator::SetLink (QString link)
{
	QString shortLink;
	Impl_->Ui_.ItemLink_->setToolTip (link);
	if (link.size () >= 40)
		shortLink = link.left (18) + "..." + link.right (18);
	else
		shortLink = link;
	if (QUrl (link).isValid ())
	{
		link.insert (0,"<a href=\"");
		link.append ("\">" + shortLink + "</a>");
		Impl_->Ui_.ItemLink_->setText (link);
	}
	else
		Impl_->Ui_.ItemLink_->setText (shortLink);
}

void Aggregator::SetCategory (const QStringList& categories)
{
	QString category = categories.join ("; ").left (60);
	if (category.isEmpty ())
	{
		Impl_->Ui_.ItemCategory_->hide ();
		Impl_->Ui_.ItemCategoryLabel_->hide ();
	}
	else
	{
		Impl_->Ui_.ItemCategory_->setText (category);
		Impl_->Ui_.ItemCategory_->show ();
		Impl_->Ui_.ItemCategoryLabel_->show ();
	}
}

void Aggregator::SetPubDate (const QDateTime& pubDate)
{
	if (pubDate.isValid ())
	{
		Impl_->Ui_.ItemPubDate_->setText (pubDate.toString ());
		Impl_->Ui_.ItemPubDate_->show ();
		Impl_->Ui_.ItemPubDateLabel_->show ();
	}
	else
	{
		Impl_->Ui_.ItemPubDate_->hide ();
		Impl_->Ui_.ItemPubDateLabel_->hide ();
	}
}

void Aggregator::SetCommentsLabel (int numComments)
{
	if (numComments >= 0)
	{
		Impl_->Ui_.ItemComments_->show ();
		Impl_->Ui_.ItemCommentsLabel_->show ();

		QString text = QString::number (numComments);
		Impl_->Ui_.ItemComments_->setText (text);
	}
	else
	{
		Impl_->Ui_.ItemComments_->hide ();
		Impl_->Ui_.ItemCommentsLabel_->hide ();
	}
}

void Aggregator::SetAuthor (const QString& itemAuthor)
{
	if (itemAuthor.isEmpty ())
	{
		Impl_->Ui_.ItemAuthor_->hide ();
		Impl_->Ui_.ItemAuthorLabel_->hide ();
	}
	else
	{
		Impl_->Ui_.ItemAuthor_->setText (itemAuthor);
		Impl_->Ui_.ItemAuthor_->show ();
		Impl_->Ui_.ItemAuthorLabel_->show ();
	}
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
		Core::Instance ().RemoveFeed (Impl_->ChannelsFilterModel_->
				mapToSource (Impl_->Ui_.Feeds_->selectionModel ()->currentIndex ()));
}

void Aggregator::on_ActionMarkItemAsUnread__triggered ()
{
    QModelIndexList indexes = Impl_->Ui_.Items_->selectionModel ()->selectedRows ();
    for (int i = 0; i < indexes.size (); ++i)
        Core::Instance ().MarkItemAsUnread (Impl_->
				ItemsFilterModel_->mapToSource (indexes.at (i)));
}

void Aggregator::on_ActionMarkChannelAsRead__triggered ()
{
    QModelIndexList indexes = Impl_->Ui_.Feeds_->selectionModel ()->selectedRows ();
    for (int i = 0; i < indexes.size (); ++i)
        Core::Instance ().MarkChannelAsRead (Impl_->
				ChannelsFilterModel_->mapToSource (indexes.at (i)));
}

void Aggregator::on_ActionMarkChannelAsUnread__triggered ()
{
    QModelIndexList indexes = Impl_->Ui_.Feeds_->selectionModel ()->selectedRows ();
    for (int i = 0; i < indexes.size (); ++i)
        Core::Instance ().MarkChannelAsUnread (Impl_->
				ChannelsFilterModel_->mapToSource (indexes.at (i)));
}

void Aggregator::on_ActionChannelSettings__triggered ()
{
    QModelIndex index = Impl_->Ui_.Feeds_->selectionModel ()->currentIndex ();
	QModelIndex mapped = Impl_->ChannelsFilterModel_->mapToSource (index);
	if (!mapped.isValid ())
		return;

	std::auto_ptr<FeedSettings> dia (new FeedSettings (mapped, this));
	dia->exec ();
}

void Aggregator::on_ActionUpdateSelectedFeed__triggered ()
{
    QModelIndex current = Impl_->Ui_.Feeds_->selectionModel ()->currentIndex ();
    if (!current.isValid ())
        return;
    Core::Instance ().UpdateFeed (Impl_->
			ChannelsFilterModel_->mapToSource (current));
}

void Aggregator::on_CaseSensitiveSearch__stateChanged (int state)
{
    Impl_->ItemsFilterModel_->setFilterCaseSensitivity (state ?
			Qt::CaseSensitive : Qt::CaseInsensitive);
}

void Aggregator::on_ActionAddToItemBucket__triggered ()
{
	Core::Instance ().AddToItemBucket (Impl_->ItemsFilterModel_->
			mapToSource (Impl_->Ui_.Items_->selectionModel ()->
				currentIndex ()));
}

void Aggregator::on_ActionItemBucket__triggered ()
{
	Impl_->ItemBucket_->show ();
}

void Aggregator::on_ActionRegexpMatcher__triggered ()
{
	Impl_->RegexpMatcherUi_->show ();
}

void Aggregator::on_ActionHideReadItems__triggered ()
{
	if (Impl_->ActionHideReadItems_->isChecked ())
		Impl_->Ui_.Items_->selectionModel ()->reset ();
	Impl_->ItemsFilterModel_->SetHideRead (Impl_->
			ActionHideReadItems_->isChecked ());
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
	Export exportDialog (tr ("Export to OPML"),
			tr ("Select save file"),
			tr ("OPML files (*.opml);;"
				"XML files (*.xml);;"
				"All files (*.*)"), this);
	channels_shorts_t channels;
	Core::Instance ().GetChannels (channels);
	exportDialog.SetFeeds (channels);
	if (exportDialog.exec () == QDialog::Rejected)
		return;

	Core::Instance ().ExportToOPML (exportDialog.GetDestination (),
			exportDialog.GetTitle (),
			exportDialog.GetOwner (),
			exportDialog.GetOwnerEmail (),
			exportDialog.GetSelectedFeeds ());
}

void Aggregator::on_ActionImportBinary__triggered ()
{
	ImportBinary import (this);
	if (import.exec () == QDialog::Rejected)
		return;

	Core::Instance ().AddFeeds (import.GetSelectedFeeds (),
			import.GetTags ());
}

void Aggregator::on_ActionExportBinary__triggered ()
{
	Export exportDialog (tr ("Export to binary file"),
			tr ("Select save file"),
			tr ("Aggregator exchange files (*.lcae);;"
				"All files (*.*)"), this);
	channels_shorts_t channels;
	Core::Instance ().GetChannels (channels);
	exportDialog.SetFeeds (channels);
	if (exportDialog.exec () == QDialog::Rejected)
		return;

	Core::Instance ().ExportToBinary (exportDialog.GetDestination (),
			exportDialog.GetTitle (),
			exportDialog.GetOwner (),
			exportDialog.GetOwnerEmail (),
			exportDialog.GetSelectedFeeds ());
}

void Aggregator::on_ItemCommentsSubscribe__released ()
{
    QModelIndex selected = Impl_->Ui_.Items_->selectionModel ()->currentIndex ();
	Core::Instance ().SubscribeToComments (Impl_->ItemsFilterModel_->
			mapToSource (selected));
}

void Aggregator::on_ItemCategoriesButton__released ()
{
	Impl_->ItemCategorySelector_->move (QCursor::pos ());
	Impl_->ItemCategorySelector_->show ();
}

void Aggregator::currentItemChanged (const QItemSelection& selection)
{
	QModelIndexList indexes = selection.indexes ();

	QModelIndex sindex;
	if (indexes.size ())
		sindex = Impl_->ItemsFilterModel_->mapToSource (indexes.at (0));

	if (!sindex.isValid () || indexes.size () != 2)
	{
		Impl_->Ui_.ItemView_->setHtml ("");
		Impl_->Ui_.ItemAuthor_->hide ();
		Impl_->Ui_.ItemAuthorLabel_->hide ();
		Impl_->Ui_.ItemCategory_->hide ();
		Impl_->Ui_.ItemCategoryLabel_->hide ();
		Impl_->Ui_.ItemLink_->setText ("");
		Impl_->Ui_.ItemPubDate_->hide ();
		Impl_->Ui_.ItemPubDateLabel_->hide ();
		Impl_->Ui_.ItemComments_->hide ();
		Impl_->Ui_.ItemCommentsLabel_->hide ();
		Impl_->Ui_.ItemCommentsSubscribe_->hide ();
		return;
	}

	Core::Instance ().Selected (sindex);

	Item_ptr item = Core::Instance ().GetItem (sindex);

	SetHtml (item->Title_, item->Description_, item->Enclosures_);
	connect (Impl_->Ui_.ItemView_->page ()->networkAccessManager (),
			SIGNAL (sslErrors (QNetworkReply*, const QList<QSslError>&)),
			&Core::Instance (),
			SLOT (handleSslError (QNetworkReply*)));

	SetAuthor (item->Author_);
	SetCategory (item->Categories_);
	SetLink (item->Link_);
	SetPubDate (item->PubDate_);
	SetCommentsLabel (item->NumComments_);

	QString commentsRSS = item->CommentsLink_;
	Impl_->Ui_.ItemCommentsSubscribe_->setVisible (!commentsRSS.isEmpty ());
}

void Aggregator::currentChannelChanged ()
{
	currentItemChanged (QItemSelection ());
	Impl_->Ui_.Items_->scrollToTop ();

    QModelIndex index = Impl_->Ui_.Feeds_->selectionModel ()->currentIndex ();
	QModelIndex mapped = Impl_->ChannelsFilterModel_->mapToSource (index);
	Core::Instance ().currentChannelChanged (mapped);

	QStringList allCategories = Core::Instance ().GetCategories (mapped);
	if (allCategories.size ())
	{
		Impl_->ItemCategorySelector_->SetPossibleSelections (allCategories);
		Impl_->ItemCategorySelector_->selectAll ();
		Impl_->Ui_.ItemCategoriesButton_->show ();
	}
	else
		Impl_->Ui_.ItemCategoriesButton_->hide ();

	Impl_->ItemsFilterModel_->categorySelectionChanged (allCategories);
}

void Aggregator::unreadNumberChanged (int number)
{
	if (!number ||
			!XmlSettingsManager::Instance ()->
				property ("ShowIconInTray").toBool ())
	{
		Impl_->TrayIcon_->hide ();
		return;
	}

	QString text = QString::number (number);

	QFont font = QApplication::font ();
	font.setPointSize (12);
	font.setFamily ("Verdana");
	QFontMetrics fm = QFontMetrics (font);
	int width = fm.width (text);
	int height = fm.height ();
	int max = std::max (width, height);

	QPixmap pixmap (":/resources/images/trayicon.png");
	pixmap = pixmap.scaled (max, max);
	QPainter painter;
	painter.begin (&pixmap);
	painter.setFont (font);
	painter.setPen (Qt::blue);
	painter.setRenderHints (QPainter::TextAntialiasing);
	painter.drawText (0, 0, width, height,
			Qt::AlignCenter, text);
	painter.end ();

	Impl_->TrayIcon_->setIcon (QIcon (pixmap));
	Impl_->TrayIcon_->show ();
}

void Aggregator::trayIconActivated ()
{
	emit bringToFront ();
	QModelIndex unread = Core::Instance ().GetUnreadChannelIndex ();
	if (unread.isValid ())
		Impl_->Ui_.Feeds_->setCurrentIndex (Impl_->ChannelsFilterModel_->
				mapFromSource (unread));
}

void Aggregator::updateItemsFilter ()
{
	int section = Impl_->Ui_.SearchType_->currentIndex ();
	QString text = Impl_->Ui_.SearchLine_->text ();
	switch (section)
	{
	case 1:
		Impl_->ItemsFilterModel_->setFilterWildcard (text);
		break;
	case 2:
		Impl_->ItemsFilterModel_->setFilterRegExp (text);
		break;
	default:
		Impl_->ItemsFilterModel_->setFilterFixedString (text);
		break;
	}
}

void Aggregator::viewerSettingsChanged ()
{
	Impl_->Ui_.ItemView_->settings ()->setFontFamily (QWebSettings::StandardFont,
			XmlSettingsManager::Instance ()->property ("StandardFont").value<QFont> ().family ());
	Impl_->Ui_.ItemView_->settings ()->setFontFamily (QWebSettings::FixedFont,
			XmlSettingsManager::Instance ()->property ("FixedFont").value<QFont> ().family ());
	Impl_->Ui_.ItemView_->settings ()->setFontFamily (QWebSettings::SerifFont,
			XmlSettingsManager::Instance ()->property ("SerifFont").value<QFont> ().family ());
	Impl_->Ui_.ItemView_->settings ()->setFontFamily (QWebSettings::SansSerifFont,
			XmlSettingsManager::Instance ()->property ("SansSerifFont").value<QFont> ().family ());
	Impl_->Ui_.ItemView_->settings ()->setFontFamily (QWebSettings::CursiveFont,
			XmlSettingsManager::Instance ()->property ("CursiveFont").value<QFont> ().family ());
	Impl_->Ui_.ItemView_->settings ()->setFontFamily (QWebSettings::FantasyFont,
			XmlSettingsManager::Instance ()->property ("FantasyFont").value<QFont> ().family ());

	Impl_->Ui_.ItemView_->settings ()->setFontSize (QWebSettings::MinimumFontSize,
			XmlSettingsManager::Instance ()->property ("MinimumFontSize").toInt ());
	Impl_->Ui_.ItemView_->settings ()->setFontSize (QWebSettings::DefaultFontSize,
			XmlSettingsManager::Instance ()->property ("DefaultFontSize").toInt ());
	Impl_->Ui_.ItemView_->settings ()->setFontSize (QWebSettings::DefaultFixedFontSize,
			XmlSettingsManager::Instance ()->property ("DefaultFixedFontSize").toInt ());
	Impl_->Ui_.ItemView_->settings ()->setAttribute (QWebSettings::AutoLoadImages,
			XmlSettingsManager::Instance ()->property ("AutoLoadImages").toBool ());
	Impl_->Ui_.ItemView_->settings ()->setAttribute (QWebSettings::JavascriptEnabled,
			XmlSettingsManager::Instance ()->property ("AllowJavaScript").toBool ());
}

void Aggregator::makeCurrentItemVisible ()
{
	QModelIndex item = Impl_->Ui_.Items_->selectionModel ()->currentIndex ();
	if (item.isValid ())
		Impl_->Ui_.Items_->scrollTo (item);
}

Q_EXPORT_PLUGIN2 (leechcraft_aggregator, Aggregator);

