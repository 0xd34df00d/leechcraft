/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "representationmanager.h"
#include <QModelIndex>
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
				.GetCurrentChannel_ = [this] { return SelectedChannel_; },
				.GetAllSelectedChannels_ = [this] { return QList { SelectedChannel_ }; },
			}) }
	, ReprWidget_ { std::make_unique<ItemsWidget> (ItemsWidget::Dependencies {
				.ShortcutsMgr_ = deps.ShortcutManager_,
				.ChannelsModel_ = deps.ChannelsModel_,
				.AppWideActions_ = deps.AppWideActions_,
				.ChannelActions_ = *ChannelActions_,
				.UpdatesManager_ = deps.UpdatesManager_,
				.ChannelNavigator_ = [this] (auto dir) { return NavigateChannel (dir); },
			})}
	, JobHolderRepresentation_ { std::make_unique<JobHolderRepresentation> (JobHolderRepresentation::Deps {
				.Toolbar_ = *ReprWidget_->GetToolBar (),
				.DetailsWidget_ = *ReprWidget_,
				.RowMenu_ = CreateMenu (*ChannelActions_, deps.AppWideActions_, *ReprWidget_),
			})}
	{
		JobHolderRepresentation_->setSourceModel (&deps.ChannelsModel_);
		ReprWidget_->ConstructBrowser ();
	}

	RepresentationManager::~RepresentationManager () = default;

	QAbstractItemModel* RepresentationManager::GetRepresentation () const
	{
		return JobHolderRepresentation_.get ();
	}

	void RepresentationManager::HandleCurrentRowChanged (const QModelIndex& index)
	{
		SelectedChannel_ = JobHolderRepresentation_->mapToSource (index);
		JobHolderRepresentation_->SelectionChanged (index);
		ReprWidget_->CurrentChannelChanged (SelectedChannel_);
	}

	bool RepresentationManager::NavigateChannel (ChannelDirection dir)
	{
		return false;
	}
}
