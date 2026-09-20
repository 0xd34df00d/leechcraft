/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "representationmanager.h"
#include <QModelIndex>
#include <QSet>
#include <util/models/modeliterator.h>
#include <util/models/selectionproxymodel.h>
#include <util/sll/prelude.h>
#include "components/actions/appwideactions.h"
#include "components/actions/channelactions.h"
#include "components/models/channelsmodel.h"
#include "components/models/jobholderrepresentationmodel.h"
#include "itemswidget.h"

namespace LC::Aggregator
{
	namespace
	{
		QMenu& CreateMenu (const ChannelActions& channelActions, const AppWideActions& appWideActions, QWidget& parent)
		{
			auto menu = new QMenu { &parent };
			menu->addActions (channelActions.GetAllActions ());
			menu->addSeparator ();
			menu->addActions (appWideActions.GetFastActions ());
			return *menu;
		}
	}

	RepresentationManager::RepresentationManager (const Deps& deps)
	: ChannelActions_ { std::make_unique<ChannelActions> (ChannelActions::Deps {
				.ShortcutManager_ = deps.ShortcutManager_,
				.UpdatesManager_ = deps.UpdatesManager_,
				.DBUpThread_ = deps.DBUpThread_,
				.GetAllSelectedChannels_ = [this] { return SelectedChannels_; },
			}) }
	, ReprWidget_ { std::make_unique<ItemsWidget> (ItemsWidget::Dependencies {
				.ShortcutsMgr_ = deps.ShortcutManager_,
				.AppWideActions_ = deps.AppWideActions_,
				.ChannelActions_ = *ChannelActions_,
				.UpdatesManager_ = deps.UpdatesManager_,
				.ChannelNavigator_ = [this] (auto dir) { NavigateChannel (dir); },
			})}
	, SelectedIdProxyModel_ { std::make_unique<SelectionProxy_t> (deps.ChannelsModel_, SelectionProxy_t::Config {
				.IsSelectedRole_ = ChannelRoles::ChannelRoleMax + 1,
				.SourceIdRole_ = ChannelRoles::ChannelID,
				.FindItems_ = std::bind_front (&ChannelsModel::FindItems, &deps.ChannelsModel_),
			})}
	, JobHolderRepresentation_ { std::make_unique<JobHolderRepresentationModel> (JobHolderRepresentationModel::Deps {
				.SelectedRole_ = SelectedIdProxyModel_->GetIsSelectedRole ()
			})}
	, RowSelector_ { deps.RowSelector_ }
	, ContextMenu_ { CreateMenu (*ChannelActions_, deps.AppWideActions_, *ReprWidget_) }
	{
		JobHolderRepresentation_->setSourceModel (&*SelectedIdProxyModel_);

		ReprWidget_->ConstructBrowser ();
	}

	RepresentationManager::~RepresentationManager () = default;

	QAbstractItemModel& RepresentationManager::GetRepresentation ()
	{
		return *JobHolderRepresentation_;
	}

	void RepresentationManager::HandleSelectedRowsChanging (const RowSelection& selection)
	{
		SelectedChannels_ = Util::Map (selection.Rows_,
				[] (const QModelIndex& idx) { return idx.data (ChannelRoles::ChannelShortStruct).value<ChannelShort> (); });
		const auto& ids = Util::Map (SelectedChannels_, &ChannelShort::ChannelID_);
		ReprWidget_->SetChannels (ids);
	}

	void RepresentationManager::HandleSelectedRowsSettled (const RowSelection& selection)
	{
		const auto& ids = Util::MapAs<QSet> (selection.Rows_,
				[] (const QModelIndex& idx) { return idx.data (ChannelRoles::ChannelID).value<IDType_t> (); });
		SelectedIdProxyModel_->SetSelections (ids);
	}

	QWidget* RepresentationManager::GetInfoWidget ()
	{
		return ReprWidget_.get ();
	}

	QToolBar* RepresentationManager::GetControls ()
	{
		return ReprWidget_->GetToolBar ();
	}

	QMenu* RepresentationManager::GetContextMenu ()
	{
		return &ContextMenu_;
	}

	namespace
	{
		std::optional<QModelIndex> FindChannelIndex (const QAbstractItemModel& model, const IDType_t& id)
		{
			for (const auto& idx : Util::AllModelRows (model))
				if (idx.data (ChannelID) == id)
					return idx;
			return {};
		}

		std::optional<QModelIndex> StepIndex (ChannelDirection dir, QModelIndex idx)
		{
			idx = idx.siblingAtRow (idx.row () + ToRowDelta (dir));
			if (!idx.isValid ())
				return {};
			return idx;
		}

		std::optional<QModelIndex> FindUnread (ChannelDirection dir, QModelIndex idx)
		{
			while (true)
			{
				if (idx.data (ChannelRoles::UnreadCount).toInt ())
					return idx;

				if (const auto step = StepIndex (dir, idx))
					idx = *step;
				else
					return {};
			}
		}
	}

	void RepresentationManager::NavigateChannel (ChannelDirection dir)
	{
		const auto rowCount = JobHolderRepresentation_->rowCount ();
		if (!rowCount)
			return;

		const auto wraparoundIdx = [&] -> QModelIndex
		{
			switch (dir)
			{
			case ChannelDirection::PreviousUnread:
				return JobHolderRepresentation_->index (rowCount - 1, 0);
			case ChannelDirection::NextUnread:
				return JobHolderRepresentation_->index (0, 0);
			}
			return {};
		} ();

		const auto id = [&] -> std::optional<IDType_t>
		{
			if (SelectedChannels_.isEmpty ())
				return {};

			switch (dir)
			{
			case ChannelDirection::NextUnread:
				return SelectedChannels_.back ().ChannelID_;
			case ChannelDirection::PreviousUnread:
				return SelectedChannels_.front ().ChannelID_;
			}
			return {};
		} ();

		const auto& row = id
				.and_then (std::bind_front (FindChannelIndex, std::ref (*JobHolderRepresentation_)))
				.and_then (std::bind_front (StepIndex, dir))
				.and_then (std::bind_front (FindUnread, dir))
				.or_else ([&] { return FindUnread (dir, wraparoundIdx); })
				;
		if (row)
			RowSelector_ (RowSelection::FromSingle (*row));
	}
}
