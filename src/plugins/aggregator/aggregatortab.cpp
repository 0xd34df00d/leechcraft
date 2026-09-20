/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "aggregatortab.h"
#include <unordered_set>
#include <QKeyEvent>
#include <QMenu>
#include <interfaces/core/icoreproxy.h>
#include <util/gui/statesaver.h>
#include <util/models/flattofoldersproxymodel.h>
#include <util/sll/qtutil.h>
#include <util/tags/tagscompleter.h>
#include <util/util.h>
#include <util/sll/prelude.h>
#include "components/actions/appwideactions.h"
#include "components/actions/channelactions.h"
#include "components/gui/util.h"
#include "components/models/channelsmodel.h"
#include "components/models/channelsfiltermodel.h"
#include "xmlsettingsmanager.h"
#include "itemswidget.h"

namespace LC::Aggregator
{
	AggregatorTab::AggregatorTab (const InitParams& deps, QObject *plugin)
	: TabClass_ { deps.TabClass_ }
	, ParentPlugin_ { plugin }
	, AppWideActions_ { deps.AppWideActions_ }
	, ChannelActions_ { std::make_unique<ChannelActions> (ChannelActions::Deps {
				.ShortcutManager_ = deps.ShortcutManager_,
				.UpdatesManager_ = deps.UpdatesManager_,
				.DBUpThread_ = deps.DBUpThread_,
				.GetAllSelectedChannels_ = [this]
				{
					return Util::Map (Ui_.Feeds_->selectionModel ()->selectedRows (),
							[] (const QModelIndex& row) { return row.data (ChannelShortStruct).value<ChannelShort> (); });
				},
			}) }
	, FlatToFolders_ { std::make_unique<Util::FlatToFoldersProxyModel> (GetProxyHolder ()->GetTagsManager ()) }
	, ChannelsFilterModel_ { new ChannelsFilterModel { this } }
	, ItemsWidget_ { std::make_unique<ItemsWidget> (ItemsWidget::Dependencies {
				.ShortcutsMgr_ = deps.ShortcutManager_,
				.AppWideActions_ = deps.AppWideActions_,
				.ChannelActions_ = *ChannelActions_,
				.UpdatesManager_ = deps.UpdatesManager_,
				.ChannelNavigator_ = [this] (auto dir) { NavigateChannel (dir); },
			}) }
	{
		ChannelsFilterModel_->setSourceModel (&deps.ChannelsModel_);

		Ui_.setupUi (this);
		Ui_.MainSplitter_->addWidget (ItemsWidget_.get ());

		FlatToFolders_->SetSourceModel (ChannelsFilterModel_);
		Ui_.Feeds_->setModel (FlatToFolders_.get ());
		connect (Ui_.Feeds_->selectionModel (),
				&QItemSelectionModel::selectionChanged,
				this,
				&AggregatorTab::CurrentChannelChanged);
		Ui_.Feeds_->expandAll ();

		connect (Ui_.Feeds_,
				&QWidget::customContextMenuRequested,
				this,
				&AggregatorTab::HandleFeedsContextMenuRequested);

		connect (Ui_.TagsLine_,
				&QLineEdit::textChanged,
				ChannelsFilterModel_,
				&ChannelsFilterModel::SetFilterString);

		new Util::TagsCompleter (Ui_.TagsLine_);
		Ui_.TagsLine_->AddSelector ();

		Ui_.MainSplitter_->setStretchFactor (0, 5);
		Ui_.MainSplitter_->setStretchFactor (1, 9);

		connect (FlatToFolders_.get (),
				&QAbstractItemModel::rowsInserted,
				Ui_.Feeds_,
				&QTreeView::expand);

		ItemsWidget_->ConstructBrowser ();

		const auto& fm = fontMetrics ();
		Util::SetupStateSaver (*Ui_.MainSplitter_,
				{
					.XSM_ = XmlSettingsManager::Instance (),
					.Id_ = "FeedsSplitter",
					.Initial_ = Util::Factors { 1, 3 },
				});
		Util::SetupStateSaver (*Ui_.Feeds_->header (),
				{
					.XSM_ = XmlSettingsManager::Instance (),
					.Id_ = "FeedsHeader",
					.Initial_ = Util::Widths { {}, fm.horizontalAdvance ("_9999_"_qs), GetDateColumnWidth (fm) },
				});
	}

	AggregatorTab::~AggregatorTab () = default;

	QToolBar* AggregatorTab::GetToolBar () const
	{
		return ItemsWidget_->GetToolBar ();
	}

	TabClassInfo AggregatorTab::GetTabClassInfo () const
	{
		return TabClass_;
	}

	QObject* AggregatorTab::ParentMultiTabs ()
	{
		return ParentPlugin_;
	}

	void AggregatorTab::Remove ()
	{
		emit removeTab ();
	}

	std::optional<TabSaveInfo> AggregatorTab::GetTabSaveInfo () const
	{
		return { { .Data_ = "aggregatortab", .Name_ = TabClass_.VisibleName_ } };
	}

	namespace
	{
		bool HasUnreadItems (const QModelIndex& index)
		{
			const auto model = index.model ();
			const auto rc = model->rowCount (index);
			if (!rc)
				return index.data (ChannelRoles::UnreadCount).toInt ();

			for (int r = 0; r < rc; ++r)
				if (HasUnreadItems (model->index (r, 0, index)))
					return true;

			return false;
		}

		int Depth (QModelIndex index)
		{
			int depth = 0;
			while ((index = index.parent ()).isValid ())
				++depth;
			return depth;
		}

		QModelIndexList RowsAtDepth (const QAbstractItemModel& model, const QModelIndex& parent, int depth)
		{
			QModelIndexList rows;
			for (int r = 0; r < model.rowCount (parent); ++r)
			{
				const auto& idx = model.index (r, 0, parent);
				if (depth)
					rows += RowsAtDepth (model, idx, depth - 1);
				else
					rows << idx;
			}
			return rows;
		}

		QModelIndexList Leaves (const QAbstractItemModel& model, const QModelIndex& parent)
		{
			QModelIndexList rows;
			for (int r = 0; r < model.rowCount (parent); ++r)
			{
				const auto& idx = model.index (r, 0, parent);
				if (model.rowCount (idx))
					rows += Leaves (model, idx);
				else
					rows << idx;
			}
			return rows;
		}
	}

	void AggregatorTab::NavigateChannel (ChannelDirection dir)
	{
		const auto& model = *Ui_.Feeds_->model ();
		const auto& current = Ui_.Feeds_->currentIndex ().siblingAtColumn (0);

		// navigate among the rows at the current row's level, or among the channels if there is none
		const auto& rows = current.isValid () ?
				RowsAtDepth (model, {}, Depth (current)) :
				Leaves (model, {});
		const auto count = rows.size ();
		const auto delta = ToRowDelta (dir);

		// wrap around, visiting every other row at most once, starting from an end if there is no current row
		const auto pos = rows.indexOf (current);
		const auto origin = pos >= 0 ? pos : (delta > 0 ? -1 : count);
		const auto steps = pos >= 0 ? count - 1 : count;
		for (qsizetype step = 1; step <= steps; ++step)
		{
			const auto& idx = rows [((origin + step * delta) % count + count) % count];
			if (HasUnreadItems (idx))
			{
				Ui_.Feeds_->setCurrentIndex (idx);
				return;
			}
		}
	}

	void AggregatorTab::HandleFeedsContextMenuRequested (const QPoint& pos)
	{
		QMenu menu;
		if (Ui_.Feeds_->indexAt (pos).isValid ())
		{
			menu.addActions (ChannelActions_->GetAllActions ());
			menu.addAction (Util::CreateSeparator (&menu));
		}
		menu.addActions (AppWideActions_.GetFastActions ());
		menu.exec (Ui_.Feeds_->viewport ()->mapToGlobal (pos));
	}

	void AggregatorTab::CurrentChannelChanged ()
	{
		QList<IDType_t> channels;

		for (const auto& index : Ui_.Feeds_->selectionModel ()->selectedRows ())
		{
			if (FlatToFolders_->IsFolder (index))
				channels << FlatToFolders_->GetChildrenData<IDType_t> (index, ChannelRoles::ChannelID);
			else
				channels << index.data (ChannelRoles::ChannelID).value<IDType_t> ();
		}

		std::unordered_set<IDType_t> duplicates;
		for (auto it = channels.begin (); it != channels.end (); )
			if (duplicates.insert (*it).second)
				++it;
			else
				it = channels.erase (it);

		ItemsWidget_->SetChannels (channels);
	}
}
