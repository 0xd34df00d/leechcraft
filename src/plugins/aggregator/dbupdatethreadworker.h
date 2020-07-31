/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <functional>
#include <optional>
#include <QObject>
#include <QVariantList>
#include <interfaces/core/ihookproxy.h>
#include <interfaces/core/icoreproxyfwd.h>
#include "common.h"
#include "channel.h"
#include "feed.h"

namespace LC
{
namespace Aggregator
{
	class StorageBackend;

	class DBUpdateThreadWorker : public QObject
	{
		Q_OBJECT

		const ICoreProxy_ptr Proxy_;
		std::shared_ptr<StorageBackend> SB_;
	public:
		DBUpdateThreadWorker (const ICoreProxy_ptr&, QObject* = nullptr);

		void WithWorker (const std::function<void (DBUpdateThreadWorker*)>&);
	private:
		Feed::FeedSettings GetFeedSettings (IDType_t);
		void AddChannel (Channel channel);
		bool AddItem (Item& item, const Channel& channel, const Feed::FeedSettings& settings);
		bool UpdateItem (const Item& item, Item ourItem);
		void NotifyUpdates (int newItems, int updatedItems, const Channel_ptr& channel);

		std::optional<IDType_t> MatchChannel (const Channel&, IDType_t, const channels_container_t&) const;
	public slots:
		void toggleChannelUnread (IDType_t channel, bool state);
		void updateFeed (channels_container_t channels, QString url);
	};
}
}
