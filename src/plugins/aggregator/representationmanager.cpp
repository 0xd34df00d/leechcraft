/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "representationmanager.h"
#include <QModelIndex>
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
				.ChannelNavigator_ = [this] (auto dir) { return NavigateChannel (dir); },
			})}
	, SelectedIdProxyModel_ { std::make_unique<SelectionProxy_t> (deps.ChannelsModel_, SelectionProxy_t::Config {
				.IsSelectedRole_ = ChannelRoles::ChannelRoleMax + 1,
				.SourceIdRole_ = ChannelRoles::ChannelID,
				.FindItems_ = std::bind_front (&ChannelsModel::FindItems, &deps.ChannelsModel_),
			})}
	, JobHolderRepresentation_ { std::make_unique<JobHolderRepresentationModel> (JobHolderRepresentationModel::Deps {
				.SelectedRole_ = SelectedIdProxyModel_->GetIsSelectedRole ()
			})}
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

	void RepresentationManager::HandleSelectedRowsChanged (const QList<QModelIndex>& indices)
	{
		SelectedChannels_ = Util::Map (indices,
				[] (const QModelIndex& idx) { return idx.data (ChannelRoles::ChannelShortStruct).value<ChannelShort> (); });
		const auto& ids = Util::Map (SelectedChannels_, &ChannelShort::ChannelID_);
		ReprWidget_->SetChannels (ids);
		SelectedIdProxyModel_->SetSelections ({ ids.begin (), ids.end () });
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

	bool RepresentationManager::NavigateChannel (ChannelDirection dir)
	{
		if (SelectedChannels_.size () != 1)
			return false;

		// TODO notify the representation view about the new index and rework the following
		return false;

		const auto& id = SelectedChannels_ [0].ChannelID_;
		for (const auto& idx : Util::AllModelRows (*JobHolderRepresentation_))
			if (idx.data (ChannelID) == id)
			{
				const auto& nextIdx = idx.siblingAtRow (idx.row () + ToRowDelta (dir));
				if (!nextIdx.isValid ())
					return false;

				HandleSelectedRowsChanged ({ nextIdx });
				return true;
			}

		return false;
	}
}
