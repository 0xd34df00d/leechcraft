/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>
#include <QModelIndex>
#include <interfaces/ijobholderrepresentationhandler.h>

namespace LC::Util
{
	class ShortcutManager;
}

namespace LC::Aggregator
{
	class ItemsWidget;
	class JobHolderRepresentation;
	class ChannelsModel;
	class UpdatesManager;
	class AppWideActions;
	class ChannelActions;
	class ResourcesFetcher;
	class DBUpdateThread;

	class RepresentationManager : public QObject
								, public IJobHolderRepresentationHandler
	{
		std::unique_ptr<ChannelActions> ChannelActions_;
		std::unique_ptr<ItemsWidget> ReprWidget_;
		std::unique_ptr<JobHolderRepresentation> JobHolderRepresentation_;

		QModelIndex SelectedChannel_;
	public:
		struct Deps
		{
			Util::ShortcutManager& ShortcutManager_;
			const AppWideActions& AppWideActions_;
			ChannelsModel& ChannelsModel_;
			UpdatesManager& UpdatesManager_;
			ResourcesFetcher& ResourcesFetcher_;
			DBUpdateThread& DBUpThread_;
		};

		explicit RepresentationManager (const Deps&);
		~RepresentationManager () override;

		QAbstractItemModel* GetRepresentation () const;

		void HandleCurrentRowChanged (const QModelIndex&) override;
	};
}
