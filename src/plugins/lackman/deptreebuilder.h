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

				struct GraphVertex;
				typedef boost::shared_ptr<GraphVertex> GraphVertex_ptr;

				struct GraphVertex
				{
					/** Makes sense only for those vertices representing
					 * packages, not any-typed dependencies, like
					 * interfaces.
					 */
					int PackageId_;

					/** Makes sense only for vertices representing any-
					 * typed dependencies, like interfaces.
					 */
					QString Dependency_;

					QList<GraphVertex_ptr> ChildVertices_;

					bool IsFulfilled_;

					/** Wheter this graph vertex is considered to be
					 * "fulfilled" when any of its dependencies is
					 * "fulfilled" or when all dependencies are
					 * fulfilled.
					 *
					 * Dependencies are represented by ChildVertices_,
					 * so basically TAny-typed node's fulfilled field
					 * is calculated as OR, and TAll — as AND.
					 *
					 * Generally, TAll-vertices are used to represent a
					 * plugin — a plugin may be installed if all its
					 * dependencies may be installed. TAny-vertices are
					 * used to represent a dependency on interface:
					 * such a dependency is fulfilled when any plugin
					 * which implements that interface may be installed.
					 */
					enum Type
					{
						TAny,//!< TAny It's enough for any child vertex to be fulfilled.
						TAll //!< TAll All child vertices should be fulfilled.
					} Type_;

					/** Constructs a not identified vertex with the
					 * given type.
					 *
					 * @param[in] type The type of this vertex.
					 */
					GraphVertex (Type type);

					/** @brief Constructs vertex with type TAll and
					 * given packageId used to identify this vertex.
					 *
					 * packageId may be later used to identify the
					 * package this vertex represents in the dependency
					 * tree.
					 *
					 * @param[in] packageId ID of the package this
					 * vertex represents.
					 */
					GraphVertex (int packageId);

					/** @brief Constructs vertex with type TAny and
					 * given dependency name used to identify this
					 * vertex.
					 *
					 * depName may be later used to identify the
					 * dependency name (like name of the interface)
					 * this vertex represents in the dependency tree.
					 *
					 * @param[in] depName Name of the dependency this
					 * vertex represents.
					 */
					GraphVertex (const QString& depName);

					void CheckFulfilled ();
				};

				GraphVertex_ptr GraphRoot_;
			public:
				DepTreeBuilder ();
				virtual ~DepTreeBuilder ();

				void BuildFor (const ListPackageInfo&);
				bool IsFulfilled () const;
			private:
				/** @brief Builds the part of dependency tree for the
				 * package identified by lpi.
				 *
				 * Once this function has finished building the layers
				 * of dep tree relevant to the package lpi, it would
				 * recursively call itself on those packages on which
				 * lpi depends.
				 *
				 * @param[in] lpi The package for which to build the
				 * tree.
				 * @param[in/out] parent The node representing lpi.
				 * It's, obviously, expected to be of type
				 * GraphNode::TAll.
				 */
				void BuildInnerLoop (const ListPackageInfo& lpi,
						GraphVertex_ptr parent);
			};
		};
	};
};

#endif
