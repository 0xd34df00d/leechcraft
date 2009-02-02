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
#include "jobholderrepresentation.h"

using LeechCraft::Util::TagsCompleter;
using LeechCraft::Util::CategorySelector;

struct Aggregator_Impl
{
    Ui::MainWidget Ui_;

	ItemsWidget *AdditionalInfo_;

	QToolBar *ToolBar_;
	QToolBar *ControlToolBar_;
    QAction *ActionAddFeed_;
    QAction *ActionUpdateFeeds_;
    QAction *ActionRemoveFeed_;
    QAction *ActionMarkChannelAsRead_;
    QAction *ActionMarkChannelAsUnread_;
	QAction *ActionChannelSettings_;
    QAction *ActionUpdateSelectedFeed_;
    QAction *ActionItemBucket_;
    QAction *ActionRegexpMatcher_;
    QAction *ActionHideReadItems_;
    QAction *ActionImportOPML_;
    QAction *ActionExportOPML_;
	QAction *ActionImportBinary_;
	QAction *ActionExportBinary_;

	std::auto_ptr<LeechCraft::Util::XmlSettingsDialog> XmlSettingsDialog_;
	std::auto_ptr<ChannelsFilterModel> ChannelsFilterModel_;
	std::auto_ptr<LeechCraft::Util::TagsCompleter> TagsLineCompleter_;
	std::auto_ptr<QSystemTrayIcon> TrayIcon_;
	std::auto_ptr<QTranslator> Translator_;
    std::auto_ptr<ItemBucket> ItemBucket_;
	std::auto_ptr<RegexpMatcherUi> RegexpMatcherUi_;

	QModelIndex SelectedRepr_;
};

Aggregator::~Aggregator ()
{
}

void Aggregator::Init ()
{
	Impl_ = new Aggregator_Impl;
	Impl_->Translator_.reset (LeechCraft::Util::InstallTranslator ("aggregator"));

	SetupActions ();

	Impl_->ToolBar_ = SetupMenuBar ();
	Impl_->ControlToolBar_ = SetupMenuBar ();
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

	Core::Instance ().DoDelayedInit ();
	Core::Instance ().GetJobHolderRepresentation ()->setParent (this);

	Impl_->AdditionalInfo_ = new ItemsWidget (this);
	Impl_->AdditionalInfo_->HideInfoPanel ();
	Impl_->Ui_.setupUi (this);

	Impl_->RegexpMatcherUi_.reset (new RegexpMatcherUi (this));

	Impl_->ItemBucket_.reset (new ItemBucket (this));
	dynamic_cast<QVBoxLayout*> (layout ())->insertWidget (0, Impl_->ToolBar_);

	Impl_->XmlSettingsDialog_.reset (new LeechCraft::Util::XmlSettingsDialog ());
	Impl_->XmlSettingsDialog_->RegisterObject (XmlSettingsManager::Instance (), ":/aggregatorsettings.xml");

	Impl_->ChannelsFilterModel_.reset (new ChannelsFilterModel (this));
	Impl_->ChannelsFilterModel_->setSourceModel (Core::Instance ().GetChannelsModel ());
	Impl_->ChannelsFilterModel_->setFilterKeyColumn (0);
	Impl_->Ui_.Feeds_->setModel (Impl_->ChannelsFilterModel_.get ());
	Impl_->Ui_.Feeds_->addAction (Impl_->ActionMarkChannelAsRead_);
	Impl_->Ui_.Feeds_->addAction (Impl_->ActionMarkChannelAsUnread_);
	QAction *sep = new QAction (Impl_->Ui_.Feeds_);
	sep->setSeparator (true);
	Impl_->Ui_.Feeds_->addAction (sep);
	Impl_->Ui_.Feeds_->addAction (Impl_->ActionChannelSettings_);
	Impl_->Ui_.Feeds_->setContextMenuPolicy (Qt::ActionsContextMenu);
	QHeaderView *channelsHeader = Impl_->Ui_.Feeds_->header ();

	QFontMetrics fm = fontMetrics ();
	int dateTimeSize = fm.width (QDateTime::currentDateTime ()
			.toString (Qt::SystemLocaleShortDate) + "__");
	channelsHeader->resizeSection (0, fm.width ("Average channel name"));
	channelsHeader->resizeSection (1, fm.width ("_9999_"));
	channelsHeader->resizeSection (2, dateTimeSize);
	connect (Impl_->Ui_.TagsLine_,
			SIGNAL (textChanged (const QString&)),
			Impl_->ChannelsFilterModel_.get (),
			SLOT (setFilterFixedString (const QString&)));
	connect (Impl_->Ui_.Feeds_->selectionModel (),
			SIGNAL (currentChanged (const QModelIndex&, const QModelIndex&)),
			this,
			SLOT (currentChannelChanged ()));
	connect (Impl_->ActionUpdateFeeds_,
			SIGNAL (triggered ()),
			&Core::Instance (),
			SLOT (updateFeeds ()));

	Impl_->TagsLineCompleter_.reset (new TagsCompleter (Impl_->Ui_.TagsLine_));
	Impl_->TagsLineCompleter_->setModel (Core::Instance ().GetTagsCompletionModel ());
	Impl_->Ui_.TagsLine_->AddSelector ();

	Impl_->Ui_.MainSplitter_->setStretchFactor (0, 5);
	Impl_->Ui_.MainSplitter_->setStretchFactor (1, 9);

	connect (&RegexpMatcherManager::Instance (),
			SIGNAL (gotLink (const QByteArray&)),
			this,
			SIGNAL (gotEntity (const QByteArray&)));

	currentChannelChanged ();
}

void Aggregator::Release ()
{
	disconnect (&Core::Instance (), 0, this, 0);
	disconnect (Impl_->ChannelsFilterModel_.get (), 0, this, 0);
	disconnect (Impl_->TagsLineCompleter_.get (), 0, this, 0);
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

QAbstractItemModel* Aggregator::GetRepresentation () const
{
	return Core::Instance ().GetJobHolderRepresentation ();
}

LeechCraft::Util::HistoryModel* Aggregator::GetHistory () const
{
	return 0;
}

QWidget* Aggregator::GetControls () const
{
	return Impl_->ControlToolBar_;
}

QWidget* Aggregator::GetAdditionalInfo () const
{
	return Impl_->AdditionalInfo_;
}

void Aggregator::ItemSelected (const QModelIndex& index)
{
	Impl_->SelectedRepr_ = index;
	QModelIndex mapped =
		Core::Instance ().GetJobHolderRepresentation ()->
			mapToSource (index);
	Core::Instance ().currentChannelChanged (mapped);
	Impl_->AdditionalInfo_->ChannelChanged (mapped);
	Impl_->Ui_.ItemsWidget_->ChannelChanged (mapped);
	Core::Instance ().GetJobHolderRepresentation ()->SelectionChanged (index);
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

QToolBar* Aggregator::SetupMenuBar ()
{
	QToolBar *bar = new QToolBar (this);

	bar->addAction (Impl_->ActionAddFeed_);
	bar->addAction (Impl_->ActionRemoveFeed_);
	bar->addAction (Impl_->ActionUpdateSelectedFeed_);
	bar->addAction (Impl_->ActionUpdateFeeds_);
	bar->addSeparator ();
	bar->addAction (Impl_->ActionItemBucket_);
	bar->addAction (Impl_->ActionRegexpMatcher_);
	bar->addSeparator ();
	bar->addAction (Impl_->ActionImportOPML_);
	bar->addAction (Impl_->ActionExportOPML_);
	bar->addAction (Impl_->ActionImportBinary_);
	bar->addAction (Impl_->ActionExportBinary_);
	bar->addSeparator ();
	bar->addAction (Impl_->ActionHideReadItems_);

	return bar;
}

void Aggregator::SetupActions ()
{
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
	bool reprMode = GetControls ()->isVisible ();
	QModelIndex ds;
	if (reprMode)
		ds = Core::Instance ().GetJobHolderRepresentation ()->
			mapToSource (Impl_->SelectedRepr_);
	else
		ds = Impl_->Ui_.Feeds_->selectionModel ()->	currentIndex ();

	QString name = ds.sibling (ds.row (), 0).data ().toString ();

	QMessageBox mb (QMessageBox::Warning,
			tr ("Warning"),
			tr ("You are going to permanently remove the feed:"
				"<br />%1<br /><br />"
				"Are you are really sure that you want to do this?",
				"Feed removing confirmation").arg (name),
			QMessageBox::Ok | QMessageBox::Cancel,
			this);
	mb.setWindowModality (Qt::WindowModal);
	if (mb.exec () == QMessageBox::Ok)
	{
		if (reprMode)
			Core::Instance ().RemoveFeed (Core::Instance ()
					.GetJobHolderRepresentation ()->
					mapToSource (Impl_->SelectedRepr_));
		else
			Core::Instance ().RemoveFeed (Impl_->ChannelsFilterModel_->
					mapToSource (Impl_->Ui_.Feeds_->selectionModel ()->
						currentIndex ()));
	}
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
	bool hide = Impl_->ActionHideReadItems_->isChecked ();
	Impl_->Ui_.ItemsWidget_->SetHideRead (hide);
	Impl_->AdditionalInfo_->SetHideRead (hide);
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

void Aggregator::currentChannelChanged ()
{
    QModelIndex index = Impl_->Ui_.Feeds_->selectionModel ()->currentIndex ();
	QModelIndex mapped = Impl_->ChannelsFilterModel_->mapToSource (index);
	Core::Instance ().currentChannelChanged (mapped);
	Impl_->Ui_.ItemsWidget_->ChannelChanged (mapped);
	Impl_->AdditionalInfo_->ChannelChanged (QModelIndex ());
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

Q_EXPORT_PLUGIN2 (leechcraft_aggregator, Aggregator);

