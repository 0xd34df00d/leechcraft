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
#include <QUrl>
#include <QToolBar>
#include <interfaces/iwebbrowser.h>
#include <util/gui/clearlineeditaddon.h>
#include <util/gui/statesaver.h>
#include <util/shortcuts/shortcutmanager.h>
#include <util/util.h>
#include <util/gui/uiinit.h>
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
#include "components/gui/util.h"
#include "components/itemrender/item.h"
#include "components/models/itemscategoriestracker.h"
#include "components/models/itemsfiltermodel.h"
#include "components/models/itemslistmodel.h"
#include "components/storage/storagebackendmanager.h"
#include "ui_itemswidget.h"
#include "xmlsettingsmanager.h"

namespace LC::Aggregator
{
	struct ItemsWidget_Impl
	{
		ItemsWidget * const Parent_;

		Ui::ItemsWidget Ui_ {};

		Util::UiInit UiInit_ { Ui_, *Parent_ };

		QToolBar *ControlToolBar_ = nullptr;

		bool TapeMode_ = XmlSettingsManager::Instance ().Property ("ShowAsTape", false).toBool ();

		const std::unique_ptr<ItemsListModel> ItemsModel_ = std::make_unique<ItemsListModel> (GetProxyHolder ()->GetIconThemeManager ());
		const ItemsCategoriesTracker CategoriesTracker_ { *ItemsModel_ };
		const std::unique_ptr<ItemsFilterModel> ItemsFilterModel_ = std::make_unique<ItemsFilterModel> (*ItemsModel_, Parent_);
		const std::unique_ptr<ItemCategorySelector> ItemCategorySelector_ = std::make_unique<ItemCategorySelector> ();
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
				deps.ChannelNavigator_,
			},
		},
		this)
	}
	{
		Impl_->ControlToolBar_ = CreateToolbar (*Actions_, deps.ChannelActions_, deps.AppWideActions_);

		Impl_->Ui_.Items_->setAcceptDrops (false);

		Impl_->ItemsFilterModel_->setFilterKeyColumn (0);
		Impl_->ItemsFilterModel_->setFilterCaseSensitivity (Qt::CaseInsensitive);
		Impl_->Ui_.Items_->setModel (Impl_->ItemsFilterModel_.get ());
		Impl_->Ui_.Items_->sortByColumn (1, Qt::DescendingOrder);

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

		new Util::ClearLineEditAddon (GetProxyHolder (), Impl_->Ui_.SearchLine_);

		Impl_->Ui_.CategoriesSplitter_->addWidget (Impl_->ItemCategorySelector_.get ());
		Util::SetupStateSaver (*Impl_->Ui_.CategoriesSplitter_,
				{ .XSM_ = XmlSettingsManager::Instance (), .Id_ = "CategoriesSplitter", .Initial_ = Util::Factors { 4, 1 } });
		connect (Impl_->ItemCategorySelector_.get (),
				&Util::CategorySelector::tagsSelectionChanged,
				Impl_->ItemsFilterModel_.get (),
				&ItemsFilterModel::InvalidateCategorySelection);
		connect (&Impl_->CategoriesTracker_,
				&ItemsCategoriesTracker::categoriesChanged,
				this,
				[this] (const QStringList& selections) { Impl_->ItemCategorySelector_->SetPossibleSelections (selections); });

		XmlSettingsManager::Instance ().RegisterObject ("ShowNavBarInItemsView", this,
				[this] (bool visible) { Impl_->Ui_.ItemView_->SetNavBarVisible (visible); });

		SelectionTracker_ = std::make_unique<ItemSelectionTracker> (*Impl_->Ui_.Items_, *Actions_, this);
		SelectionTracker_->SetTapeMode (Impl_->TapeMode_);
		connect (SelectionTracker_.get (),
				&ItemSelectionTracker::refreshItemDisplay,
				this,
				&ItemsWidget::RenderSelectedItems);
		connect (SelectionTracker_.get (),
				&ItemSelectionTracker::selectionChanged,
				Impl_->ItemsFilterModel_.get (),
				&ItemsFilterModel::InvalidateItemsSelection);

		Util::SetupStateSaver (*Impl_->Ui_.Items_->header (),
				{
					.XSM_ = XmlSettingsManager::Instance (),
					.Id_ = "ItemsHeader",
					.Initial_ = Util::Widths { {}, GetDateColumnWidth (fontMetrics ()) },
				});
		connect (Impl_->Ui_.Items_->header (),
				&QHeaderView::sectionClicked,
				this,
				&ItemsWidget::makeCurrentItemVisible);
	}

	ItemsWidget::~ItemsWidget () = default;

	QToolBar* ItemsWidget::GetToolBar () const
	{
		return Impl_->ControlToolBar_;
	}

	void ItemsWidget::SetTapeMode (bool tape)
	{
		Impl_->TapeMode_ = tape;
		SelectionTracker_->SetTapeMode (tape);
		RenderSelectedItems ();

		XmlSettingsManager::Instance ().setProperty ("ShowAsTape", tape);
	}

	void ItemsWidget::SetChannels (const QList<IDType_t>& channels)
	{
		Impl_->ItemsModel_->SetChannels (channels);

		Impl_->Ui_.Items_->scrollToTop ();
		RenderSelectedItems ();
	}

	void ItemsWidget::ConstructBrowser ()
	{
		const auto browser = GetProxyHolder ()->GetPluginsManager ()->GetAllCastableTo<IWebBrowser*> ().value (0);
		Impl_->Ui_.ItemView_->Construct (browser);
	}

	void ItemsWidget::on_CaseSensitiveSearch__stateChanged (int state)
	{
		Impl_->ItemsFilterModel_->setFilterCaseSensitivity (state ?
				Qt::CaseSensitive : Qt::CaseInsensitive);
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

	void ItemsWidget::RenderSelectedItems ()
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
	}

	void ItemsWidget::makeCurrentItemVisible ()
	{
		const auto& item = Impl_->Ui_.Items_->selectionModel ()->currentIndex ();
		if (item.isValid ())
			Impl_->Ui_.Items_->scrollTo (item);
	}

	void ItemsWidget::updateItemsFilter ()
	{
		enum class Section
		{
			FixedString = 0,
			Wildcard = 1,
			Regexp = 2,
		};

		const auto section = Impl_->Ui_.SearchType_->currentIndex ();
		const auto& text = Impl_->Ui_.SearchLine_->text ();
		switch (static_cast<Section> (section))
		{
		case Section::FixedString:
			Impl_->ItemsFilterModel_->setFilterFixedString (text);
			break;
		case Section::Wildcard:
			Impl_->ItemsFilterModel_->setFilterWildcard (text);
			break;
		case Section::Regexp:
			Impl_->ItemsFilterModel_->setFilterRegularExpression (text);
			break;
		default:
			qWarning () << "ItemsWidget::updateItemsFilter(): unknown section" << section;
			Impl_->ItemsFilterModel_->setFilterFixedString (text);
			break;
		}
	}
}
