/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "pluginsmanagerwrapper.h"

namespace LC
{
namespace Qrosp
{
	PluginsManagerWrapper::PluginsManagerWrapper (IPluginsManager *manager)
	: Manager_ (manager)
	{
	}

	QObjectList PluginsManagerWrapper::GetAllPlugins () const
	{
		return Manager_->GetAllPlugins ();
	}

	QString PluginsManagerWrapper::GetPluginLibraryPath (const QObject *object) const
	{
		return Manager_->GetPluginLibraryPath (object);
	}
}
}
