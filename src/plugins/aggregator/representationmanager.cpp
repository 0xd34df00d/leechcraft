/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "representationmanager.h"
#include <QModelIndex>
#include "jobholderrepresentation.h"
#include "itemswidget.h"
#include "channelsmodelrepresentationproxy.h"
#include "channelsmodel.h"
#include "channelactions.h"

namespace LC::Aggregator
{
	RepresentationManager::RepresentationManager (const Deps& deps)
	: ChannelActions_ { std::make_unique<ChannelActions> (ChannelActions::Deps {
				.ShortcutManager_ = deps.ShortcutManager_,
				.UpdatesManager_ = deps.UpdatesManager_,
				.ResourcesFetcher_ = deps.ResourcesFetcher_,
				.DBUpThread_ = deps.DBUpThread_,
				.GetCurrentChannel_ = [this] { return SelectedRepr_; },
				.GetAllSelectedChannels_ = [this] { return QList { SelectedRepr_ }; },
			}) }
	, ReprWidget_ { std::make_unique<ItemsWidget> () }
	, JobHolderRepresentation_ { std::make_unique<JobHolderRepresentation> () }
	, ReprModel_ { std::make_unique<ChannelsModelRepresentationProxy> () }
	{
		JobHolderRepresentation_->setSourceModel (&deps.ChannelsModel_);

		ReprWidget_->InjectDependencies ({
					.ShortcutsMgr_ = deps.ShortcutManager_,
					.ChannelsModel_ = deps.ChannelsModel_,
					.AppWideActions_ = deps.AppWideActions_,
					.ChannelActions_ = *ChannelActions_,
					.UpdatesManager_ = deps.UpdatesManager_,
				});

		ReprModel_->setSourceModel (JobHolderRepresentation_.get ());
		ReprModel_->SetWidgets (ReprWidget_->GetToolBar (), ReprWidget_.get ());
		// TODO ReprModel_->SetMenu (CreateFeedsContextMenu (deps.ChannelActions_, deps.AppWideActions_));

		ReprWidget_->ConstructBrowser ();
	}

	RepresentationManager::~RepresentationManager () = default;

	QAbstractItemModel* RepresentationManager::GetRepresentation () const
	{
		return ReprModel_.get ();
	}

	void RepresentationManager::HandleCurrentRowChanged (const QModelIndex& srcIdx)
	{
		auto index = ReprModel_->mapToSource (srcIdx);
		SelectedRepr_ = JobHolderRepresentation_->SelectionChanged (index);
		ReprWidget_->CurrentChannelChanged (SelectedRepr_);
	}
}
