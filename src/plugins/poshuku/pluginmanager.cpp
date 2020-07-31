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
#include "proxyobject.h"
#include "core.h"

namespace LC
{
namespace Poshuku
{
	PluginManager::PluginManager (QObject *parent)
	: Util::BaseHookInterconnector (parent)
	, ProxyObject_ (new ProxyObject)
	{
	}

	void PluginManager::AddPlugin (QObject *plugin)
	{
		if (plugin->metaObject ()->indexOfMethod (QMetaObject::normalizedSignature ("initPlugin (QObject*)")) != -1)
			QMetaObject::invokeMethod (plugin,
					"initPlugin",
					Q_ARG (QObject*, ProxyObject_.get ()));

		Util::BaseHookInterconnector::AddPlugin (plugin);
	}
}
}
