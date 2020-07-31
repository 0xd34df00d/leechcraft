/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "pluginmanager.h"

namespace LC
{
namespace Poshuku
{
namespace OnlineBookmarks
{
	PluginManager::PluginManager (QObject *parent)
	: Util::BaseHookInterconnector (parent)
	{
	}
	
	void PluginManager::AddPlugin (QObject *plugin)
	{
		Util::BaseHookInterconnector::AddPlugin (plugin);
	}
}
}
}


