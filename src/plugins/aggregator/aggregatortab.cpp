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
				.GetCurrentChannel_ = [this] () -> std::optional<ChannelShort>
				{
					if (const auto& idx = Ui_.Feeds_->currentIndex (); idx.isValid ())
						return idx.data (ChannelShortStruct).value<ChannelShort> ();
					return {};
				},
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
				.ChannelNavigator_ = [this] (auto dir) { return NavigateChannel (dir); },
			}) }
	{
		ChannelsFilterModel_->setSourceModel (&deps.ChannelsModel_);
		ChannelsFilterModel_->setFilterKeyColumn (0);

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
				&QSortFilterProxyModel::setFilterFixedString);

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

	QByteArray AggregatorTab::GetTabRecoverData () const
	{
		return "aggregatortab";
	}

	QIcon AggregatorTab::GetTabRecoverIcon () const
	{
		return TabClass_.Icon_;
	}

	QString AggregatorTab::GetTabRecoverName () const
	{
		return TabClass_.VisibleName_;
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

		QModelIndex NavigateSibling (ChannelDirection dir, QModelIndex index)
		{
			const auto delta = ToRowDelta (dir);
			do
			{
				index = index.siblingAtRow (index.row () + delta);
			} while (index.isValid () && !HasUnreadItems (index));
			return index;
		}

		QModelIndex NavigateViaParent (ChannelDirection dir, QModelIndex parent)
		{
			parent = NavigateSibling (dir, parent);
			if (!parent.isValid ())
				return {};

			const auto model = parent.model ();

			QModelIndex child;
			switch (dir)
			{
			case ChannelDirection::NextUnread:
				child = model->index (0, 0, parent);
				break;
			case ChannelDirection::PreviousUnread:
				child = model->index (model->rowCount (parent) - 1, 0, parent);
				break;
			}

			return HasUnreadItems (child) ? child : NavigateSibling (dir, child);
		}
	}

	bool AggregatorTab::NavigateChannel (ChannelDirection dir)
	{
		const auto& index = Ui_.Feeds_->currentIndex ();

		auto newIndex = NavigateSibling (dir, index);
		if (!newIndex.isValid () && index.parent ().isValid ())
			newIndex = NavigateViaParent (dir, index.parent ());

		if (!newIndex.isValid ())
			return false;

		Ui_.Feeds_->setCurrentIndex (newIndex);
		return true;
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
