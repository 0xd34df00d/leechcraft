/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Georg Rudoy
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

#include "qrosp.h"
#include <QIcon>
#include "pluginmanager.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Qrosp
		{
			void Plugin::Init (ICoreProxy_ptr proxy)
			{
			}

			void Plugin::SecondInit ()
			{
			}

			void Plugin::Release ()
			{
				PluginManager::Instance ().Release ();
			}

			QString Plugin::GetName () const
			{
				return "Qrosp";
			}

			QString Plugin::GetInfo () const
			{
				return tr ("Makes LeechCraft scriptable using Qross.");
			}

			QIcon Plugin::GetIcon () const
			{
				return QIcon ();
			}

			QStringList Plugin::Provides () const
			{
				return QStringList ();
			}

			QStringList Plugin::Needs () const
			{
				return QStringList ();
			}

			QStringList Plugin::Uses () const
			{
				return QStringList ();
			}

			void Plugin::SetProvider (QObject*, const QString&)
			{
			}

			QList<QObject*> Plugin::GetPlugins ()
			{
				return PluginManager::Instance ().GetPlugins ();
			}
		};
	};
};

Q_EXPORT_PLUGIN2 (leechcraft_qrosp, LeechCraft::Plugins::Qrosp::Plugin);

