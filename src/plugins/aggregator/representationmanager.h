/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <optional>
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
	class ChannelsModelRepresentationProxy;
	class ChannelsModel;
	struct AppWideActions;
	struct ChannelActions;
	struct ItemsWidgetDependencies;

	class RepresentationManager : public QObject
								, public IJobHolderRepresentationHandler
	{
		ItemsWidget *ReprWidget_ = nullptr;
		JobHolderRepresentation *JobHolderRepresentation_ = nullptr;
		ChannelsModelRepresentationProxy *ReprModel_ = nullptr;
		QModelIndex SelectedRepr_;
	public:
		struct InitParams
		{
			const AppWideActions& AppWideActions_;
			const ChannelActions& ChannelActions_;
			ChannelsModel *ChannelsModel_;

			const ItemsWidgetDependencies& ReprWidgetDeps_;
		};

		explicit RepresentationManager (const InitParams&);

		QAbstractItemModel* GetRepresentation () const;

		void HandleCurrentRowChanged (const QModelIndex&) override;

		std::optional<QModelIndex> GetRelevantIndex () const;
	};
}
