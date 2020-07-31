/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QVariant>
#include <interfaces/core/ihookproxy.h>
#include <util/xpc/basehookinterconnector.h>
#include "interfaces/aggregator/item.h"
#include "proxyobject.h"

namespace LC
{
namespace Aggregator
{
	class ChannelsModel;

	class PluginManager : public Util::BaseHookInterconnector
	{
		Q_OBJECT

		std::shared_ptr<ProxyObject> ProxyObject_;
	public:
		explicit PluginManager (ChannelsModel*, QObject* = nullptr);

		void AddPlugin (QObject*) override;
	signals:
		void hookItemLoad (LC::IHookProxy_ptr proxy,
				Item*);
		void hookItemAdded (LC::IHookProxy_ptr proxy,
				const Item& item);
	};
}
}
