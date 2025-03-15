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
#include "channel.h"

namespace LC::Util
{
	class ShortcutManager;

	template<typename>
	class SelectionProxyModel;
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
	enum class ChannelDirection;

	class RepresentationManager : public QObject
								, public IJobHolderRepresentationHandler
	{
		using SelectionProxy_t = Util::SelectionProxyModel<IDType_t>;

		const std::unique_ptr<ChannelActions> ChannelActions_;
		const std::unique_ptr<ItemsWidget> ReprWidget_;
		const std::unique_ptr<SelectionProxy_t> SelectedIdProxyModel_;
		const std::unique_ptr<JobHolderRepresentation> JobHolderRepresentation_;

		std::optional<ChannelShort> CurrentChannel_;
		QList<ChannelShort> SelectedChannels_;
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

		void HandleCurrentRowChanged(const QModelIndex&) override;
		void HandleSelectedRowsChanged (const QList<QModelIndex>&) override;
	private:
		bool NavigateChannel (ChannelDirection);
	};
}
