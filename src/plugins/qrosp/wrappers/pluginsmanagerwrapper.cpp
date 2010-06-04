/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2010  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "pluginsmanagerwrapper.h"

namespace LeechCraft
{
	namespace Plugins
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
		};
	};
};
