/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "pluginmanager.h"
#include "interfaces/aggregator/iaggregatorplugin.h"

namespace LC
{
namespace Aggregator
{
	PluginManager::PluginManager (ChannelsModel *cm, QObject *parent)
	: Util::BaseHookInterconnector (parent)
	, ProxyObject_ (new ProxyObject (cm))
	{
	}

	void PluginManager::AddPlugin (QObject *plugin)
	{
		if (const auto iap = qobject_cast<IAggregatorPlugin*> (plugin))
			iap->InitPlugin (ProxyObject_.get ());

		Util::BaseHookInterconnector::AddPlugin (plugin);
	}
}
}
