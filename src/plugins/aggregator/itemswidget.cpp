/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "itemswidget.h"
#include <memory>
#include <algorithm>
#include <QHeaderView>
#include <QSortFilterProxyModel>
#include <QUrl>
#include <QTimer>
#include <QMessageBox>
#include <QToolBar>
#include <QtDebug>
#include <interfaces/iwebbrowser.h>
#include <util/models/mergemodel.h>
#include <util/gui/clearlineeditaddon.h>
#include <util/shortcuts/shortcutmanager.h>
#include <util/util.h>
#include <util/sll/containerconversions.h>
#include <util/sll/qtutil.h>
#include <interfaces/core/itagsmanager.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/core/ientitymanager.h>
#include "components/actions/appwideactions.h"
#include "components/actions/channelactions.h"
#include "components/actions/itemactions.h"
#include "components/gui/itemcategoryselector.h"
#include "components/gui/itemnavigator.h"
#include "components/gui/itemselectiontracker.h"
#include "components/itemrender/item.h"
#include "ui_itemswidget.h"
#include "xmlsettingsmanager.h"
#include "itemsfiltermodel.h"
#include "itemslistmodel.h"
#include "channelsmodel.h"
#include "uistatepersist.h"
#include "storagebackendmanager.h"
#include "itemutils.h"

namespace LC::Aggregator
{
	struct ItemsWidget_Impl
	{
		ItemsWidget * const Parent_;

		Ui::ItemsWidget Ui_ {};

		struct UiInit
		{
			UiInit (auto&& ui, QWidget *parent) { ui.setupUi (parent); }
		} UiInit_ { Ui_, Parent_ };

		QToolBar *ControlToolBar_ = nullptr;

		bool TapeMode_ = false;
		bool MergeMode_ = false;

		QAbstractItemModel *ChannelsModel_ = nullptr;

		std::unique_ptr<ItemsListModel> CurrentItemsModel_ {};
		QList<std::shared_ptr<ItemsListModel>> SupplementaryModels_ {};
		std::unique_ptr<Util::MergeModel> ItemLists_ {};
		const std::unique_ptr<ItemsFilterModel> ItemsFilterModel_ = std::make_unique<ItemsFilterModel> (Parent_);
		std::unique_ptr<ItemCategorySelector> ItemCategorySelector_ {};

		QTimer *SelectedChecker_ = nullptr;

		// The last selected index into the ItemLists_ model.
		QModelIndex LastSelectedIndex_ {};

		QModelIndex LastSelectedChannel_ {};

		UpdatesManager *UpdatesManager_ = nullptr;
	};

	namespace
	{
		auto CreateToolbar (const ItemActions& itemActs,
				const ChannelActions& channelActs,
				const AppWideActions& appWideActs)
		{
			const auto toolbar = new QToolBar {};
			toolbar->addActions (appWideActs.GetFastActions ());
			toolbar->addSeparator ();
			toolbar->addActions (channelActs.GetToolbarActions ());
			toolbar->addSeparator ();
			toolbar->addActions (itemActs.GetToolbarActions ());
			return toolbar;
		}
	}

	ItemsWidget::ItemsWidget (const Dependencies& deps, QWidget *parent)
	: QWidget (parent)
	, Impl_ (new ItemsWidget_Impl { this })
	, Actions_ { std::make_unique<ItemActions> (ItemActions::Deps {
			.Parent_ = this,
			.ShortcutsMgr_ = deps.ShortcutsMgr_,
			.UpdatesManager_ = deps.UpdatesManager_,
			.SetHideRead_ = [this] (bool hide) { Impl_->ItemsFilterModel_->SetHideRead (hide); },
			.SetShowTape_ = [this] (bool tape) { SetTapeMode (tape); },
			.GetSelection_ = [this] { return Impl_->Ui_.Items_->selectionModel ()->selectedRows (); },
			.ItemNavigator_ = ItemNavigator
			{
				*Impl_->Ui_.Items_,
				Impl_->LastSelectedIndex_,
				[this] (const QModelIndex& newChan)
				{
					emit movedToChannel (newChan);
					CurrentChannelChanged (newChan);
				}
			},
		},
		this)
	}
	{
		Impl_->SelectedChecker_ = new QTimer (this);
		Impl_->SelectedChecker_->setSingleShot (true);
		connect (Impl_->SelectedChecker_,
				&QTimer::timeout,
				this,
				&ItemsWidget::checkSelected);

		Impl_->UpdatesManager_ = &deps.UpdatesManager_;

		auto& cm = deps.ChannelsModel_;
		Impl_->ChannelsModel_ = &cm;
		connect (&cm,
				&QAbstractItemModel::rowsInserted,
				this,
				&ItemsWidget::invalidateMergeMode);
		connect (&cm,
				&QAbstractItemModel::rowsRemoved,
				this,
				&ItemsWidget::invalidateMergeMode);

		Impl_->TapeMode_ = XmlSettingsManager::Instance ()->Property ("ShowAsTape", false).toBool ();

		Impl_->ControlToolBar_ = CreateToolbar (*Actions_, deps.ChannelActions_, deps.AppWideActions_);

		const auto& proxy = GetProxyHolder ();
		Impl_->CurrentItemsModel_ = std::make_unique<ItemsListModel> (proxy->GetIconThemeManager ());
		Impl_->ItemLists_ = std::make_unique<Util::MergeModel> (QStringList { tr ("Name"), tr ("Date") });
		Impl_->ItemLists_->AddModel (Impl_->CurrentItemsModel_.get ());

		Impl_->Ui_.Items_->setAcceptDrops (false);

		Impl_->ItemsFilterModel_->SetItemsWidget (this);
		Impl_->ItemsFilterModel_->setSourceModel (Impl_->ItemLists_.get ());
		Impl_->ItemsFilterModel_->setFilterKeyColumn (0);
		Impl_->ItemsFilterModel_->setFilterCaseSensitivity (Qt::CaseInsensitive);
		Impl_->Ui_.Items_->setModel (Impl_->ItemsFilterModel_.get ());
		Impl_->Ui_.Items_->sortByColumn (1, Qt::DescendingOrder);
		connect (Impl_->ItemLists_.get (),
				&QAbstractItemModel::dataChanged,
				Impl_->ItemsFilterModel_.get (),
				&QSortFilterProxyModel::invalidate);

		Impl_->Ui_.Items_->addActions (Actions_->GetAllActions ());
		Impl_->Ui_.Items_->setContextMenuPolicy (Qt::ActionsContextMenu);

		addActions (Actions_->GetInvisibleActions ());

		connect (Impl_->Ui_.SearchLine_,
				&QLineEdit::textChanged,
				this,
				&ItemsWidget::updateItemsFilter);
		connect (Impl_->Ui_.SearchType_,
				qOverload<int> (&QComboBox::currentIndexChanged),
				this,
				&ItemsWidget::updateItemsFilter);

		new Util::ClearLineEditAddon (proxy, Impl_->Ui_.SearchLine_);

		const auto header = Impl_->Ui_.Items_->header ();
		const auto& fm = fontMetrics ();
		header->resizeSection (0,
				fm.horizontalAdvance ("Average news article size is about this width or maybe bigger, because they are bigger"));
		header->resizeSection (1,
				fm.horizontalAdvance (QLocale {}.toString (QDateTime::currentDateTime (), QLocale::ShortFormat) + "__"));
		connect (Impl_->Ui_.Items_->header (),
				&QHeaderView::sectionClicked,
				this,
				&ItemsWidget::makeCurrentItemVisible);

		Impl_->ItemCategorySelector_ = std::make_unique<ItemCategorySelector> ();
		Impl_->Ui_.CategoriesSplitter_->addWidget (Impl_->ItemCategorySelector_.get ());
		connect (Impl_->ItemCategorySelector_.get (),
				&Util::CategorySelector::tagsSelectionChanged,
				Impl_->ItemsFilterModel_.get (),
				&ItemsFilterModel::categorySelectionChanged);

		XmlSettingsManager::Instance ()->RegisterObject ("ShowNavBarInItemsView", this,
				[this] (bool visible) { Impl_->Ui_.ItemView_->SetNavBarVisible (visible); });

		SelectionTracker_ = std::make_unique<ItemSelectionTracker> (*Impl_->Ui_.Items_, *Actions_, this);
		connect (SelectionTracker_.get (),
				&ItemSelectionTracker::refreshItemDisplay,
				this,
				&ItemsWidget::currentItemChanged);
	}

	ItemsWidget::~ItemsWidget ()
	{
		on_CategoriesSplitter__splitterMoved ();

		disconnect (Impl_->ItemsFilterModel_.get (),
				0,
				this,
				0);
		delete Impl_;
	}

	QToolBar* ItemsWidget::GetToolBar () const
	{
		return Impl_->ControlToolBar_;
	}

	QModelIndex ItemsWidget::GetUnfilteredSelectedIndex () const
	{
		return Impl_->LastSelectedIndex_;
	}

	void ItemsWidget::SetTapeMode (bool tape)
	{
		Impl_->TapeMode_ = tape;
		SelectionTracker_->SetItemDependsOnSelection (!tape);
		currentItemChanged ();

		XmlSettingsManager::Instance ()->setProperty ("ShowAsTape", tape);
	}

	void ItemsWidget::SetMergeMode (bool merge)
	{
		Impl_->MergeMode_ = merge;
		ClearSupplementaryModels ();

		if (Impl_->MergeMode_)
		{
			auto cm = Impl_->ChannelsModel_;
			for (int i = 0, size = cm->rowCount (); i < size; ++i)
			{
				auto index = cm->index (i, 0);
				AddSupplementaryModelFor (index.data (ChannelRoles::ChannelID).value<IDType_t> ());
			}
		}
	}

	void ItemsWidget::SetMergeModeTags (const QStringList& tags)
	{
		if (Impl_->MergeMode_)
			return;

		ClearSupplementaryModels ();

		const auto& tagsSet = Util::AsSet (tags);

		bool added = false;

		const auto cm = Impl_->ChannelsModel_;
		for (int i = 0, size = cm->rowCount (); i < size; ++i)
		{
			const auto& index = cm->index (i, 0);
			const auto& thisSet = index.data (RoleTags).toStringList ();
			if (std::none_of (thisSet.begin (), thisSet.end (),
					[&tagsSet] (const QString& tag) { return tagsSet.contains (tag); }))
				continue;

			auto cid = index.data (ChannelRoles::ChannelID).value<IDType_t> ();

			/** So that first one gets assigned to the
			 * current items model.
			 */
			if (!added)
			{
				Impl_->CurrentItemsModel_->Reset (cid);
				added = true;
			}
			else
				AddSupplementaryModelFor (cid);
		}
	}

	void ItemsWidget::Selected (const QModelIndex& index)
	{
		Impl_->LastSelectedIndex_ = index;

		Impl_->SelectedChecker_->stop ();
		const auto timeout = XmlSettingsManager::Instance ()->
				property ("MarkAsReadTimeout").toInt () * 1000;
		Impl_->SelectedChecker_->start (timeout);
	}

	void ItemsWidget::CurrentChannelChanged (const QModelIndex& si)
	{
		if (Impl_->MergeMode_)
			return;

		ClearSupplementaryModels ();

		Impl_->LastSelectedChannel_ = si;

		if (si.isValid ())
			Impl_->CurrentItemsModel_->Reset (si.data (ChannelRoles::ChannelID).value<IDType_t> ());
		else
			Impl_->CurrentItemsModel_->Reset (IDNotFound);

		Impl_->Ui_.Items_->scrollToTop ();
		currentItemChanged ();

		if (!isVisible ())
			return;

		const auto& items = Impl_->CurrentItemsModel_->GetAllItems ();
		const auto& allCategories = ItemUtils::GetCategories (items).values ();
		Impl_->ItemCategorySelector_->SetPossibleSelections (allCategories);
	}

	void ItemsWidget::ConstructBrowser ()
	{
		const auto browser = GetProxyHolder ()->GetPluginsManager ()->GetAllCastableTo<IWebBrowser*> ().value (0);
		Impl_->Ui_.ItemView_->Construct (browser);
	}

	void ItemsWidget::LoadUIState ()
	{
		LoadColumnWidth (Impl_->Ui_.Items_, "items");
	}

	void ItemsWidget::SaveUIState ()
	{
		SaveColumnWidth (Impl_->Ui_.Items_, "items");
	}

	void ItemsWidget::ClearSupplementaryModels ()
	{
		while (Impl_->SupplementaryModels_.size ())
		{
			Impl_->ItemLists_->RemoveModel (Impl_->SupplementaryModels_.at (0).get ());
			Impl_->SupplementaryModels_.removeAt (0);
		}
	}

	void ItemsWidget::AddSupplementaryModelFor (IDType_t channelId)
	{
		if (channelId == Impl_->CurrentItemsModel_->GetCurrentChannel ())
			return;

		auto ilm = std::make_shared<ItemsListModel> (GetProxyHolder ()->GetIconThemeManager ());
		ilm->Reset (channelId);
		Impl_->SupplementaryModels_ << ilm;
		Impl_->ItemLists_->AddModel (ilm.get ());
	}

	void ItemsWidget::RestoreSplitter ()
	{
		QList<int> sizes;
		sizes << XmlSettingsManager::Instance ()->
			Property ("CategoriesSplitter1", 0).toInt ();
		sizes << XmlSettingsManager::Instance ()->
			Property ("CategoriesSplitter2", 0).toInt ();
		if (!sizes.at (0) &&
				!sizes.at (1))
		{
			Impl_->Ui_.CategoriesSplitter_->setStretchFactor (0, 8);
			Impl_->Ui_.CategoriesSplitter_->setStretchFactor (1, 1);
		}
		else
			Impl_->Ui_.CategoriesSplitter_->setSizes (sizes);
	}

	void ItemsWidget::invalidateMergeMode ()
	{
		if (Impl_->MergeMode_)
		{
			SetMergeMode (false);
			SetMergeMode (true);
		}
	}

	void ItemsWidget::on_CaseSensitiveSearch__stateChanged (int state)
	{
		Impl_->ItemsFilterModel_->setFilterCaseSensitivity (state ?
				Qt::CaseSensitive : Qt::CaseInsensitive);
	}

	void ItemsWidget::on_CategoriesSplitter__splitterMoved ()
	{
		QList<int> sizes = Impl_->Ui_.CategoriesSplitter_->sizes ();
		XmlSettingsManager::Instance ()->
			setProperty ("CategoriesSplitter1", sizes.at (0));
		XmlSettingsManager::Instance ()->
			setProperty ("CategoriesSplitter2", sizes.at (1));
	}

	namespace
	{
		QVector<IDType_t> GetAllDisplayedItems (const QAbstractItemModel& model)
		{
			QVector<IDType_t> result;
			const auto size = model.rowCount ();
			result.reserve (size);
			for (int i = 0; i < size; ++ i)
				result << model.index (i, 0).data (IItemsModel::ItemRole::ItemId).value<IDType_t> ();
			return result;
		}

		QVector<IDType_t> ToItemIds (const QModelIndexList& idxes)
		{
			QVector<IDType_t> result;
			result.reserve (idxes.size ());
			for (const auto& idx : idxes)
				result << idx.data (IItemsModel::ItemRole::ItemId).value<IDType_t> ();
			return result;
		}
	}

	void ItemsWidget::currentItemChanged ()
	{
		const auto sb = StorageBackendManager::Instance ().MakeStorageBackendForThread ();
		const auto& itemsToDisplay = Impl_->TapeMode_ ?
				GetAllDisplayedItems (*Impl_->ItemsFilterModel_) :
				ToItemIds (Impl_->Ui_.Items_->selectionModel ()->selectedRows ());

		const auto preHtml = R"(<html><head><meta charset="UTF-8" /><title>News</title></head><body bgcolor=")"_qs +
				palette ().color (QPalette::Base).name () +
				"\">"_qs;

		QString html;
		QUrl base;
		for (const auto itemId : itemsToDisplay)
			if (const auto& item = sb->GetItem (itemId))
			{
				html += ItemToHtml (*item);
				if (base.isEmpty ())
					base = item->Link_;
			}
		Impl_->Ui_.ItemView_->SetHtml (preHtml + html + "</body></html>"_qs, base);

		if (Impl_->TapeMode_)
		{
			auto sourceIndex = Impl_->Ui_.Items_->currentIndex ();
			if (sourceIndex.isValid ())
			{
				const auto& cIndex = Impl_->ItemsFilterModel_->mapToSource (sourceIndex);
				Selected (cIndex);
			}
		}
	}

	void ItemsWidget::checkSelected ()
	{
		const auto& sourceIndex = Impl_->Ui_.Items_->currentIndex ();
		const auto& cIndex = Impl_->ItemsFilterModel_->mapToSource (sourceIndex);
		if (cIndex != Impl_->LastSelectedIndex_ || cIndex.data (IItemsModel::ItemRole::IsRead).toBool ())
			return;

		const auto sb = StorageBackendManager::Instance ().MakeStorageBackendForThread ();
		sb->SetItemUnread (cIndex.data (IItemsModel::ItemRole::ItemId).value<IDType_t> (), false);
	}

	void ItemsWidget::makeCurrentItemVisible ()
	{
		const auto& item = Impl_->Ui_.Items_->selectionModel ()->currentIndex ();
		if (item.isValid ())
			Impl_->Ui_.Items_->scrollTo (item);
	}

	namespace
	{
		enum SearchSection
		{
			FixedString = 0,
			Wildcard = 1,
			Regexp = 2,
			ImportantThisChannel = 3,
			ImportantAllChannels = 4,
		};
	}

	void ItemsWidget::updateItemsFilter ()
	{
		const int section = Impl_->Ui_.SearchType_->currentIndex ();
		if (section == SearchSection::ImportantAllChannels)
		{
			const auto& sb = StorageBackendManager::Instance ().MakeStorageBackendForThread ();
			Impl_->CurrentItemsModel_->Reset (sb->GetItemsForTag ("_important"));
		}
		else
			CurrentChannelChanged (Impl_->LastSelectedChannel_);

		const QString& text = Impl_->Ui_.SearchLine_->text ();
		switch (section)
		{
		case SearchSection::FixedString:
			Impl_->ItemsFilterModel_->setFilterFixedString (text);
			break;
		case SearchSection::Wildcard:
			Impl_->ItemsFilterModel_->setFilterWildcard (text);
			break;
		case SearchSection::Regexp:
			Impl_->ItemsFilterModel_->setFilterRegExp (text);
			break;
		default:
			qWarning () << "ItemsWidget::updateItemsFilter(): unknown section" << section;
			Impl_->ItemsFilterModel_->setFilterFixedString (text);
			break;
		}

		QList<ITagsManager::tag_id> tags;
		if (section == SearchSection::ImportantThisChannel)
			tags << "_important";
		Impl_->ItemsFilterModel_->SetItemTags (tags);
	}
}
