/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "pluginmanager.h"
#include <stdexcept>
#include <QtDebug>
#include "core.h"

namespace LC
{
namespace Azoth
{
	PluginManager::PluginManager (QObject *parent)
	: Util::BaseHookInterconnector (parent)
	{
	}
}
}
