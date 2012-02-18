/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include <QMessageBox>
#include <QtDebug>
#include <QSortFilterProxyModel>
#include <QHeaderView>
#include <QCompleter>
#include <QSystemTrayIcon>
#include <QPainter>
#include <QMenu>
#include <QToolBar>
#include <QQueue>
#include <QTimer>
#include <QTranslator>
#include <QCursor>
#include <QKeyEvent>
#include <interfaces/entitytesthandleresult.h>
#include <interfaces/core/icoreproxy.h>
#include <util/tagscompletionmodel.h>
#include <util/util.h>
#include <util/categoryselector.h>
#include <util/tagscompleter.h>
#include <util/backendselector.h>
#include <util/flattofoldersproxymodel.h>
#include <util/shortcuts/shortcutmanager.h>
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
#include "regexpmatcherui.h"
#include "regexpmatchermanager.h"
#include "export.h"
#include "importbinary.h"
#include "feedsettings.h"
#include "jobholderrepresentation.h"
#include "wizardgenerator.h"
#include "export2fb2dialog.h"
#include "actionsstructs.h"
#include "uistatepersist.h"
#include "itemswidget.h"

namespace LeechCraft
{
namespace Aggregator
{
	using LeechCraft::Util::TagsCompleter;
	using LeechCraft::Util::CategorySelector;
	using LeechCraft::ActionInfo;

	struct Aggregator_Impl
	{
		Ui::MainWidget Ui_;

		AppWideActions AppWideActions_;
		ChannelActions ChannelActions_;

		Util::ShortcutManager *ShortcutMgr_;

		QMenu *ToolMenu_;

		std::shared_ptr<Util::FlatToFoldersProxyModel> FlatToFolders_;
		std::shared_ptr<Util::XmlSettingsDialog> XmlSettingsDialog_;
		std::unique_ptr<Util::TagsCompleter> TagsLineCompleter_;
		std::unique_ptr<QSystemTrayIcon> TrayIcon_;
		std::unique_ptr<RegexpMatcherUi> RegexpMatcherUi_;

		QModelIndex SelectedRepr_;

		TabClassInfo TabInfo_;

		bool InitFailed_;
	};

	Aggregator::~Aggregator ()
	{
	}

	void Aggregator::Init (ICoreProxy_ptr proxy)
	{
		setProperty ("IsUnremoveable", true);

		Impl_ = new Aggregator_Impl;
		Impl_->InitFailed_ = false;
		Util::InstallTranslator ("aggregator");

		Impl_->TabInfo_.TabClass_ = "Aggregator";
		Impl_->TabInfo_.VisibleName_ = GetName ();
		Impl_->TabInfo_.Description_ = GetInfo ();
		Impl_->TabInfo_.Icon_ = GetIcon ();
		Impl_->TabInfo_.Priority_ = 0;
		Impl_->TabInfo_.Features_ = TabFeatures (TFSingle | TFOpenableByRequest);
		Impl_->ShortcutMgr_ = new Util::ShortcutManager (proxy, this);

		Impl_->ChannelActions_.SetupActionsStruct (this);
		Impl_->AppWideActions_.SetupActionsStruct (this);
		Core::Instance ().SetAppWideActions (Impl_->AppWideActions_);

		Impl_->ToolMenu_ = new QMenu (tr ("Aggregator"));
		Impl_->ToolMenu_->setIcon (GetIcon ());
		Impl_->ToolMenu_->addAction (Impl_->AppWideActions_.ActionImportOPML_);
		Impl_->ToolMenu_->addAction (Impl_->AppWideActions_.ActionExportOPML_);
		Impl_->ToolMenu_->addAction (Impl_->AppWideActions_.ActionImportBinary_);
		Impl_->ToolMenu_->addAction (Impl_->AppWideActions_.ActionExportBinary_);
		Impl_->ToolMenu_->addAction (Impl_->AppWideActions_.ActionExportFB2_);

		Impl_->TrayIcon_.reset (new QSystemTrayIcon (QIcon (":/resources/images/aggregator.svg"), this));
		Impl_->TrayIcon_->hide ();
		connect (Impl_->TrayIcon_.get (),
				SIGNAL (activated (QSystemTrayIcon::ActivationReason)),
				this,
				SLOT (trayIconActivated ()));

		Core::Instance ().SetProxy (proxy);

		connect (&Core::Instance (),
				SIGNAL (unreadNumberChanged (int)),
				this,
				SLOT (unreadNumberChanged (int)));
		connect (&Core::Instance (),
				SIGNAL (delegateEntity (const LeechCraft::Entity&,
						int*, QObject**)),
				this,
				SIGNAL (delegateEntity (const LeechCraft::Entity&,
						int*, QObject**)));

		Impl_->XmlSettingsDialog_.reset (new LeechCraft::Util::XmlSettingsDialog ());
		Impl_->XmlSettingsDialog_->RegisterObject (XmlSettingsManager::Instance (),
				"aggregatorsettings.xml");
		Impl_->XmlSettingsDialog_->SetCustomWidget ("BackendSelector",
				new LeechCraft::Util::BackendSelector (XmlSettingsManager::Instance ()));

		if (!Core::Instance ().DoDelayedInit ())
		{
			setEnabled (false);
			Impl_->AppWideActions_.ActionAddFeed_->setEnabled (false);
			Impl_->AppWideActions_.ActionUpdateFeeds_->setEnabled (false);
			Impl_->AppWideActions_.ActionRegexpMatcher_->setEnabled (false);
			Impl_->AppWideActions_.ActionImportOPML_->setEnabled (false);
			Impl_->AppWideActions_.ActionExportOPML_->setEnabled (false);
			Impl_->AppWideActions_.ActionImportBinary_->setEnabled (false);
			Impl_->AppWideActions_.ActionExportBinary_->setEnabled (false);
			Impl_->AppWideActions_.ActionExportFB2_->setEnabled (false);
			Impl_->InitFailed_ = true;
			qWarning () << Q_FUNC_INFO
				<< "core initialization failed";
		}

		Impl_->Ui_.setupUi (this);
		Impl_->Ui_.ItemsWidget_->SetAppWideActions (Impl_->AppWideActions_);
		Impl_->Ui_.ItemsWidget_->SetChannelActions (Impl_->ChannelActions_);

		if (Impl_->InitFailed_)
		{
			QMessageBox::critical (this,
					"LeechCraft",
					tr ("Aggregator failed to initialize properly. "
						"Check logs and talk with the developers. "
						"Or, at least, check the storage backend "
						"settings and restart LeechCraft."));
			return;
		}

		Impl_->Ui_.ItemsWidget_->
			SetChannelsFilter (Core::Instance ()
					.GetChannelsModel ());
		Core::Instance ().GetJobHolderRepresentation ()->setParent (this);
		Core::Instance ().GetReprWidget ()->SetAppWideActions (Impl_->AppWideActions_);
		Core::Instance ().GetReprWidget ()->SetChannelActions (Impl_->ChannelActions_);

		Impl_->Ui_.MergeItems_->setChecked (XmlSettingsManager::Instance ()->
				Property ("MergeItems", false).toBool ());

		Impl_->RegexpMatcherUi_.reset (new RegexpMatcherUi (this));

		Impl_->FlatToFolders_.reset (new Util::FlatToFoldersProxyModel);
		Impl_->FlatToFolders_->SetTagsManager (Core::Instance ().GetProxy ()->GetTagsManager ());
		handleGroupChannels ();
		connect (Impl_->FlatToFolders_.get (),
				SIGNAL (rowsInserted (const QModelIndex&,
						int, int)),
				Impl_->Ui_.Feeds_,
				SLOT (expandAll ()));
		connect (Impl_->FlatToFolders_.get (),
				SIGNAL (rowsRemoved (const QModelIndex&,
						int, int)),
				Impl_->Ui_.Feeds_,
				SLOT (expandAll ()));
		XmlSettingsManager::Instance ()->RegisterObject ("GroupChannelsByTags",
				this, "handleGroupChannels");

		Impl_->Ui_.Feeds_->addAction (Impl_->
				ChannelActions_.ActionMarkChannelAsRead_);
		Impl_->Ui_.Feeds_->addAction (Impl_->
				ChannelActions_.ActionMarkChannelAsUnread_);
		Impl_->Ui_.Feeds_->addAction (Util::CreateSeparator (Impl_->Ui_.Feeds_));
		Impl_->Ui_.Feeds_->addAction (Impl_->
				ChannelActions_.ActionRemoveFeed_);
		Impl_->Ui_.Feeds_->addAction (Impl_->
				ChannelActions_.ActionUpdateSelectedFeed_);
		Impl_->Ui_.Feeds_->addAction (Util::CreateSeparator (Impl_->Ui_.Feeds_));
		Impl_->Ui_.Feeds_->addAction (Impl_->
				ChannelActions_.ActionRemoveChannel_);
		Impl_->Ui_.Feeds_->addAction (Util::CreateSeparator (Impl_->Ui_.Feeds_));
		Impl_->Ui_.Feeds_->addAction (Impl_->
				ChannelActions_.ActionChannelSettings_);
		Impl_->Ui_.Feeds_->addAction (Util::CreateSeparator (Impl_->Ui_.Feeds_));
		Impl_->Ui_.Feeds_->addAction (Impl_->
				AppWideActions_.ActionAddFeed_);
		connect (Impl_->Ui_.Feeds_,
				SIGNAL (customContextMenuRequested (const QPoint&)),
				this,
				SLOT (handleFeedsContextMenuRequested (const QPoint&)));
		QHeaderView *channelsHeader = Impl_->Ui_.Feeds_->header ();

		QMenu *contextMenu = new QMenu (tr ("Feeds actions"));
		contextMenu->addAction (Impl_->
				ChannelActions_.ActionMarkChannelAsRead_);
		contextMenu->addAction (Impl_->
				ChannelActions_.ActionMarkChannelAsUnread_);
		contextMenu->addSeparator ();
		contextMenu->addAction (Impl_->
				ChannelActions_.ActionRemoveFeed_);
		contextMenu->addAction (Impl_->
				ChannelActions_.ActionUpdateSelectedFeed_);
		contextMenu->addSeparator ();
		contextMenu->addAction (Impl_->
				ChannelActions_.ActionChannelSettings_);
		contextMenu->addSeparator ();
		contextMenu->addAction (Impl_->
				AppWideActions_.ActionAddFeed_);
		Core::Instance ().SetContextMenu (contextMenu);

		QFontMetrics fm = fontMetrics ();
		int dateTimeSize = fm.width (QDateTime::currentDateTime ()
				.toString (Qt::SystemLocaleShortDate) + "__");
		channelsHeader->resizeSection (0, fm.width ("Average channel name"));
		channelsHeader->resizeSection (1, fm.width ("_9999_"));
		channelsHeader->resizeSection (2, dateTimeSize);
		connect (Impl_->Ui_.TagsLine_,
				SIGNAL (textChanged (const QString&)),
				Core::Instance ().GetChannelsModel (),
				SLOT (setFilterFixedString (const QString&)));
		connect (Impl_->AppWideActions_.ActionUpdateFeeds_,
				SIGNAL (triggered ()),
				&Core::Instance (),
				SLOT (updateFeeds ()));

		Impl_->TagsLineCompleter_.reset (new TagsCompleter (Impl_->Ui_.TagsLine_));
		Impl_->Ui_.TagsLine_->AddSelector ();

		Impl_->Ui_.MainSplitter_->setStretchFactor (0, 5);
		Impl_->Ui_.MainSplitter_->setStretchFactor (1, 9);

		connect (&RegexpMatcherManager::Instance (),
				SIGNAL (gotLink (const LeechCraft::Entity&)),
				this,
				SIGNAL (gotEntity (const LeechCraft::Entity&)));
		connect (&Core::Instance (),
				SIGNAL (gotEntity (const LeechCraft::Entity&)),
				this,
				SIGNAL (gotEntity (const LeechCraft::Entity&)));

		currentChannelChanged ();

		BuildID2ActionTupleMap ();
	}

	void Aggregator::SecondInit ()
	{
		LoadColumnWidth (Impl_->Ui_.Feeds_, "feeds");

		if (Impl_->InitFailed_)
			return;

		Impl_->Ui_.ItemsWidget_->ConstructBrowser ();
		Impl_->Ui_.ItemsWidget_->LoadUIState ();

		Core::Instance ().GetReprWidget ()->ConstructBrowser ();
	}

	void Aggregator::Release ()
	{
		SaveColumnWidth (Impl_->Ui_.Feeds_, "feeds");
		Impl_->Ui_.ItemsWidget_->SaveUIState ();
		disconnect (&Core::Instance (), 0, this, 0);
		if (Core::Instance ().GetChannelsModel ())
			disconnect (Core::Instance ().GetChannelsModel (), 0, this, 0);
		if (Impl_->TagsLineCompleter_.get ())
			disconnect (Impl_->TagsLineCompleter_.get (), 0, this, 0);
		Impl_->TrayIcon_->hide ();
		delete Impl_;
		Core::Instance ().Release ();
	}

	QByteArray Aggregator::GetUniqueID () const
	{
		return "org.LeechCraft.Aggregator";
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

	void Aggregator::SetProvider (QObject*, const QString&)
	{
	}

	QIcon Aggregator::GetIcon () const
	{
		return QIcon (":/resources/images/aggregator.svg");
	}

	TabClasses_t Aggregator::GetTabClasses () const
	{
		TabClasses_t result;
		result << Impl_->TabInfo_;
		return result;
	}

	QToolBar* Aggregator::GetToolBar () const
	{
		return Impl_->Ui_.ItemsWidget_->GetToolBar ();
	}

	void Aggregator::TabOpenRequested (const QByteArray& tabClass)
	{
		if (tabClass == "Aggregator")
			emit addNewTab (GetTabClassInfo ().VisibleName_, this);
		else
			qWarning () << Q_FUNC_INFO
					<< "unknown tab class"
					<< tabClass;
	}

	TabClassInfo Aggregator::GetTabClassInfo () const
	{
		return Impl_->TabInfo_;
	}

	QObject* Aggregator::ParentMultiTabs ()
	{
		return this;
	}

	void Aggregator::Remove ()
	{
		emit removeTab (this);
	}

	std::shared_ptr<LeechCraft::Util::XmlSettingsDialog> Aggregator::GetSettingsDialog () const
	{
		return Impl_->XmlSettingsDialog_;
	}

	QAbstractItemModel* Aggregator::GetRepresentation () const
	{
		return Core::Instance ().GetJobHolderRepresentation ();
	}

	void Aggregator::handleTasksTreeSelectionCurrentRowChanged (const QModelIndex& index, const QModelIndex&)
	{
		QModelIndex si = Core::Instance ().GetProxy ()->MapToSource (index);
		if (si.model () != GetRepresentation ())
			si = QModelIndex ();
		si = Core::Instance ().GetJobHolderRepresentation ()->SelectionChanged (si);
		Impl_->SelectedRepr_ = si;
		Core::Instance ().GetReprWidget ()->CurrentChannelChanged (si);
	}

	EntityTestHandleResult Aggregator::CouldHandle (const LeechCraft::Entity& e) const
	{
		EntityTestHandleResult r;
		if (Core::Instance ().CouldHandle (e))
			r.HandlePriority_ = 1000;
		return r;
	}

	void Aggregator::Handle (LeechCraft::Entity e)
	{
		Core::Instance ().Handle (e);
	}

	void Aggregator::SetShortcut (const QString& name,
			const QKeySequences_t& shortcuts)
	{
		Impl_->ShortcutMgr_->SetShortcut (name, shortcuts);
	}

	QMap<QString, ActionInfo> Aggregator::GetActionInfo () const
	{
		return Impl_->ShortcutMgr_->GetActionInfo ();
	}

	QList<QWizardPage*> Aggregator::GetWizardPages () const
	{
		return WizardGenerator ().GetPages ();
	}

	QList<QAction*> Aggregator::GetActions (ActionsEmbedPlace place) const
	{
		QList<QAction*> result;

		switch (place)
		{
		case AEPToolsMenu:
			result << Impl_->ToolMenu_->menuAction ();
			result << Impl_->AppWideActions_.ActionRegexpMatcher_;
			break;
		case AEPCommonContextMenu:
			result << Impl_->AppWideActions_.ActionAddFeed_;
			result << Impl_->AppWideActions_.ActionUpdateFeeds_;
			break;
		case AEPTrayMenu:
			result << Impl_->AppWideActions_.ActionAddFeed_;
			result << Impl_->AppWideActions_.ActionUpdateFeeds_;
			break;
		case AEPQuickLaunch:
			break;
		default:
			qWarning () << Q_FUNC_INFO
					<< "unknown place"
					<< place;
		}

		return result;
	}

	QSet<QByteArray> Aggregator::GetExpectedPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Aggregator.GeneralPlugin/1.0";
		return result;
	}

	void Aggregator::AddPlugin (QObject *plugin)
	{
		Core::Instance ().AddPlugin (plugin);
	}

	Sync::ChainIDs_t Aggregator::AvailableChains () const
	{
		Sync::ChainIDs_t result;
		result << "rss";
		return result;
	}

	Sync::Payloads_t Aggregator::GetAllDeltas (const Sync::ChainID_t& chain) const
	{
		return Sync::Payloads_t ();
	}

	Sync::Payloads_t Aggregator::GetNewDeltas (const Sync::ChainID_t& chain) const
	{
		return Sync::Payloads_t ();
	}

	void Aggregator::PurgeNewDeltas (const Sync::ChainID_t& chain, quint32 since)
	{
	}

	void Aggregator::ApplyDeltas (const Sync::Payloads_t& payloads, const Sync::ChainID_t& chain)
	{
	}

	void Aggregator::keyPressEvent (QKeyEvent *e)
	{
		if (e->modifiers () & Qt::ControlModifier)
		{
			QItemSelectionModel *channelSM = Impl_->Ui_.Feeds_->selectionModel ();
			QModelIndex currentChannel = channelSM->currentIndex ();
			int numChannels = Impl_->Ui_.Feeds_->
				model ()->rowCount (currentChannel.parent ());

			QItemSelectionModel::SelectionFlags chanSF =
				QItemSelectionModel::Select |
				QItemSelectionModel::Clear |
				QItemSelectionModel::Rows;

			if (e->key () == Qt::Key_Less &&
					currentChannel.isValid ())
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
					currentChannel.isValid ())
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
					!currentChannel.isValid ())
			{
				QModelIndex next = Impl_->Ui_.Feeds_->model ()->index (0, 0);
				channelSM->select (next, chanSF);
				channelSM->setCurrentIndex (next, chanSF);
			}
		}
		e->ignore ();
	}

	bool Aggregator::IsRepr () const
	{
		return Core::Instance ().GetReprWidget ()->isVisible ();
	}

	QModelIndex Aggregator::GetRelevantIndex () const
	{
		if (IsRepr ())
			return Core::Instance ()
					.GetJobHolderRepresentation ()->
							mapToSource (Impl_->SelectedRepr_);
		else
		{
			QModelIndex index = Impl_->Ui_.Feeds_->
					selectionModel ()->currentIndex ();
			if (Impl_->FlatToFolders_->GetSourceModel ())
				index = Impl_->FlatToFolders_->MapToSource (index);
			return Core::Instance ().GetChannelsModel ()->mapToSource (index);
		}
	}

	QList<QModelIndex> Aggregator::GetRelevantIndexes () const
	{
		if (IsRepr ())
		{
			QList<QModelIndex> result;
			result << Core::Instance ()
					.GetJobHolderRepresentation ()->
							mapToSource (Impl_->SelectedRepr_);
			return result;
		}

		QList<QModelIndex> result;
		Q_FOREACH (QModelIndex index,
				Impl_->Ui_.Feeds_->selectionModel ()->selectedRows ())
		{
			if (Impl_->FlatToFolders_->GetSourceModel ())
				index = Impl_->FlatToFolders_->MapToSource (index);
			result << Core::Instance ().GetChannelsModel ()->mapToSource (index);
		}
		return result;
	}

	void Aggregator::BuildID2ActionTupleMap ()
	{
		typedef Util::ShortcutManager::IDPair_t ID_t;
		*Impl_->ShortcutMgr_ << ID_t ("ActionAddFeed", Impl_->AppWideActions_.ActionAddFeed_)
				<< ID_t ("ActionUpdateFeeds_", Impl_->AppWideActions_.ActionUpdateFeeds_)
				<< ID_t ("ActionRegexpMatcher_", Impl_->AppWideActions_.ActionRegexpMatcher_)
				<< ID_t ("ActionImportOPML_", Impl_->AppWideActions_.ActionImportOPML_)
				<< ID_t ("ActionExportOPML_", Impl_->AppWideActions_.ActionExportOPML_)
				<< ID_t ("ActionImportBinary_", Impl_->AppWideActions_.ActionImportBinary_)
				<< ID_t ("ActionExportBinary_", Impl_->AppWideActions_.ActionExportBinary_)
				<< ID_t ("ActionExportFB2_", Impl_->AppWideActions_.ActionExportFB2_)
				<< ID_t ("ActionRemoveFeed_", Impl_->ChannelActions_.ActionRemoveFeed_)
				<< ID_t ("ActionUpdateSelectedFeed_", Impl_->ChannelActions_.ActionUpdateSelectedFeed_)
				<< ID_t ("ActionMarkChannelAsRead_", Impl_->ChannelActions_.ActionMarkChannelAsRead_)
				<< ID_t ("ActionMarkChannelAsUnread_", Impl_->ChannelActions_.ActionMarkChannelAsUnread_)
				<< ID_t ("ActionChannelSettings_", Impl_->ChannelActions_.ActionChannelSettings_);
	}

	void Aggregator::on_ActionAddFeed__triggered ()
	{
		AddFeed af (QString (), this);
		if (af.exec () == QDialog::Accepted)
			Core::Instance ().AddFeed (af.GetURL (), af.GetTags ());
	}

	void Aggregator::on_ActionRemoveFeed__triggered ()
	{
		QModelIndex ds = GetRelevantIndex ();

		if (!ds.isValid ())
			return;

		QString name = ds.sibling (ds.row (), 0).data ().toString ();

		QMessageBox mb (QMessageBox::Warning,
				"LeechCraft",
				tr ("You are going to permanently remove the feed:"
					"<br />%1<br /><br />"
					"Are you really sure that you want to do it?",
					"Feed removal confirmation").arg (name),
				QMessageBox::Ok | QMessageBox::Cancel,
				this);
		mb.setWindowModality (Qt::WindowModal);
		if (mb.exec () == QMessageBox::Ok)
			Core::Instance ().RemoveFeed (ds);
	}

	void Aggregator::on_ActionRemoveChannel__triggered ()
	{
		QModelIndex ds = GetRelevantIndex ();

		if (!ds.isValid ())
			return;

		QString name = ds.sibling (ds.row (), 0).data ().toString ();

		QMessageBox mb (QMessageBox::Warning,
				"LeechCraft",
				tr ("You are going to remove the channel:"
					"<br />%1<br /><br />"
					"Are you really sure that you want to do it?",
					"Channel removal confirmation").arg (name),
				QMessageBox::Ok | QMessageBox::Cancel,
				this);
		mb.setWindowModality (Qt::WindowModal);
		if (mb.exec () == QMessageBox::Ok)
			Core::Instance ().RemoveChannel (ds);
	}

	void Aggregator::Perform (boost::function<void (const QModelIndex&)> func)
	{
		QList<QModelIndex> indexes = GetRelevantIndexes ();
		Q_FOREACH (QModelIndex index, indexes)
		{
			if (index.isValid ())
				func (index);
			else if (Impl_->FlatToFolders_->GetSourceModel ())
			{
				index = Impl_->Ui_.Feeds_->
						selectionModel ()->currentIndex ();
				for (int i = 0, size = Impl_->FlatToFolders_->rowCount (index);
					i < size; ++i)
					{
						QModelIndex source = Impl_->FlatToFolders_->index (i, 0, index);
						source = Impl_->FlatToFolders_->MapToSource (source);
						func (source);
					}
			}
		}
	}

	void Aggregator::on_ActionMarkChannelAsRead__triggered ()
	{
		Perform ([] (const QModelIndex& mi) { Core::Instance ().MarkChannelAsRead (mi); });
	}

	void Aggregator::on_ActionMarkChannelAsUnread__triggered ()
	{
		Perform ([] (const QModelIndex& mi) { Core::Instance ().MarkChannelAsUnread (mi); });
	}

	void Aggregator::on_ActionChannelSettings__triggered ()
	{
		QModelIndex index = GetRelevantIndex ();
		if (!index.isValid ())
			return;

		std::unique_ptr<FeedSettings> dia (new FeedSettings (index, this));
		dia->exec ();
	}

	void Aggregator::handleFeedsContextMenuRequested (const QPoint& pos)
	{
		bool enable = Impl_->Ui_.Feeds_->indexAt (pos).isValid ();
		QList<QAction*> toToggle;
		toToggle << Impl_->ChannelActions_.ActionMarkChannelAsRead_
				<< Impl_->ChannelActions_.ActionMarkChannelAsUnread_
				<< Impl_->ChannelActions_.ActionRemoveFeed_
				<< Impl_->ChannelActions_.ActionChannelSettings_
				<< Impl_->ChannelActions_.ActionUpdateSelectedFeed_;

		Q_FOREACH (QAction *act, toToggle)
			act->setEnabled (enable);

		QMenu *menu = new QMenu;
		menu->setAttribute (Qt::WA_DeleteOnClose, true);
		menu->addActions (Impl_->Ui_.Feeds_->actions ());
		menu->exec (Impl_->Ui_.Feeds_->viewport ()->mapToGlobal (pos));

		Q_FOREACH (QAction *act, toToggle)
			act->setEnabled (true);
	}

	void Aggregator::on_ActionUpdateSelectedFeed__triggered ()
	{
		const bool repr = IsRepr ();
		Perform ([repr] (const QModelIndex& mi) { Core::Instance ().UpdateFeed (mi, repr); });
	}

	void Aggregator::on_ActionRegexpMatcher__triggered ()
	{
		Impl_->RegexpMatcherUi_->show ();
	}

	void Aggregator::on_ActionImportOPML__triggered ()
	{
		Core::Instance ().StartAddingOPML (QString ());
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

	void Aggregator::on_ActionExportFB2__triggered ()
	{
		Export2FB2Dialog *dialog = new Export2FB2Dialog (this);
		connect (dialog,
				SIGNAL (gotEntity (const LeechCraft::Entity&)),
				this,
				SIGNAL (gotEntity (const LeechCraft::Entity&)));
		dialog->setAttribute (Qt::WA_DeleteOnClose);
		dialog->show ();
	}

	void Aggregator::on_MergeItems__toggled (bool merge)
	{
		Impl_->Ui_.ItemsWidget_->SetMergeMode (merge);
		XmlSettingsManager::Instance ()->setProperty ("MergeItems", merge);
	}

	void Aggregator::currentChannelChanged ()
	{
		QModelIndex index = Impl_->Ui_.Feeds_->
				selectionModel ()->currentIndex ();
		if (Impl_->FlatToFolders_->GetSourceModel ())
		{
			QModelIndex origIndex = index;
			index = Impl_->FlatToFolders_->MapToSource (index);
			if (!index.isValid ())
			{
				QStringList tags = origIndex.data (RoleTags).toStringList ();
				Impl_->Ui_.ItemsWidget_->SetMergeModeTags (tags);
				return;
			}
		}
		Impl_->Ui_.ItemsWidget_->CurrentChannelChanged (index);
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

		QString tip = tr ("%n unread message(s)", "", number) + " " +
					tr ("in %n channel(s).", "", Core::Instance ().GetUnreadChannelsNumber ());
		Impl_->TrayIcon_->setToolTip (tip);
		Impl_->TrayIcon_->show ();
	}

	void Aggregator::trayIconActivated ()
	{
		emit raiseTab (this);
		QModelIndex unread = Core::Instance ().GetUnreadChannelIndex ();
		if (unread.isValid ())
		{
			if (Impl_->FlatToFolders_->GetSourceModel ())
				unread = Impl_->FlatToFolders_->MapFromSource (unread).at (0);
			Impl_->Ui_.Feeds_->setCurrentIndex (unread);
		}
	}

	void Aggregator::handleGroupChannels ()
	{
		if (XmlSettingsManager::Instance ()->
				property ("GroupChannelsByTags").toBool ())
		{
			Impl_->FlatToFolders_->SetSourceModel (Core::Instance ().GetChannelsModel ());
			Impl_->Ui_.Feeds_->setModel (Impl_->FlatToFolders_.get ());
		}
		else
		{
			Impl_->FlatToFolders_->SetSourceModel (0);
			Impl_->Ui_.Feeds_->setModel (Core::Instance ().GetChannelsModel ());
		}
		connect (Impl_->Ui_.Feeds_->selectionModel (),
				SIGNAL (currentChanged (const QModelIndex&, const QModelIndex&)),
				this,
				SLOT (currentChannelChanged ()));
		Impl_->Ui_.Feeds_->expandAll ();
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_aggregator, LeechCraft::Aggregator::Aggregator);
