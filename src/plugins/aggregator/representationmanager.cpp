/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "representationmanager.h"
#include "jobholderrepresentation.h"
#include "itemswidget.h"
#include "channelsmodelrepresentationproxy.h"
#include "channelsmodel.h"
#include "actionsstructs.h"

namespace LC::Aggregator
{
	RepresentationManager::RepresentationManager (const InitParams& params)
	{
		JobHolderRepresentation_ = new JobHolderRepresentation ();
		JobHolderRepresentation_->setSourceModel (params.ChannelsModel_);

		ReprWidget_ = new ItemsWidget;
		ReprWidget_->InjectDependencies (params.ReprWidgetDeps_);

		ReprModel_ = new ChannelsModelRepresentationProxy { this };
		ReprModel_->setSourceModel (JobHolderRepresentation_);
		ReprModel_->SetWidgets (ReprWidget_->GetToolBar (), ReprWidget_);
		ReprModel_->SetMenu (CreateFeedsContextMenu (params.ChannelActions_, params.AppWideActions_));

		ReprWidget_->ConstructBrowser ();
	}

	QAbstractItemModel* RepresentationManager::GetRepresentation () const
	{
		return ReprModel_;
	}

	void RepresentationManager::HandleCurrentRowChanged (const QModelIndex& srcIdx)
	{
		auto index = ReprModel_->mapToSource (srcIdx);
		index = JobHolderRepresentation_->SelectionChanged (index);
		SelectedRepr_ = index;
		ReprWidget_->CurrentChannelChanged (index);
	}

	std::optional<QModelIndex> RepresentationManager::GetRelevantIndex () const
	{
		if (!ReprWidget_->isVisible ())
			return {};

		return JobHolderRepresentation_->mapToSource (SelectedRepr_);
	}
}
