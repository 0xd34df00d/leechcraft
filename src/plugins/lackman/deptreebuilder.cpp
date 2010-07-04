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

#include "deptreebuilder.h"
#include "core.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LackMan
		{
			DepTreeBuilder::DepTreeBuilder ()
			{
			}

			DepTreeBuilder::~DepTreeBuilder ()
			{
			}

			void DepTreeBuilder::operator() (const ListPackageInfo& packageInfo)
			{
				QList<Dependency> dependencies = Core::Instance ().GetDependencies (packageInfo.PackageID_);

				Q_FOREACH (const Dependency& dep, dependencies)
				{
					if (Core::Instance ().IsFulfilled (dep))
						continue;

					QList<ListPackageInfo> suitable = Core::Instance ().GetDependencyFulfillers (dep);
				}
			}
		};
	};
};
