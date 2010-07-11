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

#ifndef PLUGINS_LACKMAN_DEPTREEBUILDER_H
#define PLUGINS_LACKMAN_DEPTREEBUILDER_H
#include <boost/shared_ptr.hpp>
#include <QStack>
#include "repoinfo.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LackMan
		{
			class DepTreeBuilder
			{
			public:
				typedef QStack<QString> DFSLevels_t;
			private:
				/** List of package names that are parents of the
				 * package currently being visited.
				 */
				QStack<QString> Levels_;

				struct GraphNode;
				typedef boost::shared_ptr<GraphNode> GraphNode_ptr;

				struct GraphNode
				{
					int PackageId_;
					QList<GraphNode_ptr> ChildNodes_;

					/** Wheter this graph node is considered to be
					 * "fulfilled" when any of its dependencies is
					 * "fulfilled" or when all dependencies are
					 * fulfilled.
					 *
					 * Dependencies are represented by ChildNodes_, so
					 * basically TAny-typed node's fulfilled field is
					 * calculated as OR, and TAll — as AND.
					 *
					 * Generally, TAll-nodes are used to represent a
					 * plugin — a plugin may be installed if all its
					 * dependencies may be installed. TAny-nodes are
					 * used to represent a dependency on interface:
					 * such a dependency is fulfilled when any plugin
					 * which implements that interface may be installed.
					 */
					enum Type
					{
						TAny,//!< TAny It's enough for any child node to be fulfilled.
						TAll //!< TAll All child nodes should be fulfilled.
					} Type_;

					GraphNode (Type);
				};

				GraphNode_ptr GraphRoot_;
			public:
				DepTreeBuilder ();
				virtual ~DepTreeBuilder ();

				void BuildFor (const ListPackageInfo&);
			private:
				void BuildInnerLoop (const ListPackageInfo&);
			};
		};
	};
};

#endif
