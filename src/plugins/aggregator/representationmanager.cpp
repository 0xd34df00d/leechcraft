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
#include "jobholderrepresentation.h"
#include "itemswidget.h"
#include "channelsmodel.h"

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
				.ResourcesFetcher_ = deps.ResourcesFetcher_,
				.DBUpThread_ = deps.DBUpThread_,
				.GetCurrentChannel_ = [this] { return CurrentChannel_; },
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
	, JobHolderRepresentation_ { std::make_unique<JobHolderRepresentation> (JobHolderRepresentation::Deps {
				.Toolbar_ = *ReprWidget_->GetToolBar (),
				.DetailsWidget_ = *ReprWidget_,
				.RowMenu_ = CreateMenu (*ChannelActions_, deps.AppWideActions_, *ReprWidget_),
				.SelectedRole_ = SelectedIdProxyModel_->GetIsSelectedRole (),
			})}
	{
		JobHolderRepresentation_->setSourceModel (&*SelectedIdProxyModel_);
		ReprWidget_->ConstructBrowser ();
	}

	RepresentationManager::~RepresentationManager () = default;

	QAbstractItemModel* RepresentationManager::GetRepresentation () const
	{
		return JobHolderRepresentation_.get ();
	}

	void RepresentationManager::HandleCurrentRowChanged (const QModelIndex& index)
	{
		if (index.isValid ())
		{
			CurrentChannel_ = index.data (ChannelRoles::ChannelShortStruct).value<ChannelShort> ();
			SelectedIdProxyModel_->SetSelections ({ CurrentChannel_->ChannelID_ });
		}
		else
		{
			CurrentChannel_.reset ();
			SelectedIdProxyModel_->SetSelections ({});
		}
	}

	void RepresentationManager::HandleSelectedRowsChanged (const QList<QModelIndex>& indices)
	{
		SelectedChannels_ = Util::Map (indices,
				[] (const QModelIndex& idx) { return idx.data (ChannelRoles::ChannelShortStruct).value<ChannelShort> (); });
		ReprWidget_->SetChannels (Util::Map (SelectedChannels_, &ChannelShort::ChannelID_));
	}

	bool RepresentationManager::NavigateChannel (ChannelDirection dir)
	{
		if (!CurrentChannel_)
			return false;

		for (const auto& idx : Util::AllModelRows (*JobHolderRepresentation_))
			if (idx.data (ChannelID) == CurrentChannel_->ChannelID_)
			{
				const auto& nextIdx = idx.siblingAtRow (idx.row () + ToRowDelta (dir));
				if (!nextIdx.isValid ())
					return false;

				// TODO notify the representation view about the new index
				HandleCurrentRowChanged (nextIdx);
				return true;
			}

		return false;
	}
}
