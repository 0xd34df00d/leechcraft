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
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/properties.hpp>
#include <QHash>
#include <QStack>
#include "repoinfo.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LackMan
		{
			struct CycleDetector;
			struct FulfillableChecker;
			struct VertexPredicate;

			class DepTreeBuilder
			{
				struct VertexInfo
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

					/** Constructs an empty VertexInfo of type TAny.
					 */
					VertexInfo ();

					/** Constructs a not identified vertex with the
					 * given type.
					 *
					 * @param[in] type The type of this vertex.
					 */
					VertexInfo (Type type);

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
					VertexInfo (int packageId);

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
					VertexInfo (const QString& depName);
				};

				typedef boost::shared_ptr<VertexInfo> VertexInfo_ptr;

				typedef boost::property<boost::vertex_color_t, boost::default_color_type,
						VertexInfo> VertexProperty;
				typedef boost::adjacency_list<boost::vecS, boost::vecS,
						boost::bidirectionalS, VertexProperty> Graph_t;

				typedef Graph_t::vertex_descriptor Vertex_t;
				typedef Graph_t::edge_descriptor Edge_t;
				typedef Graph_t::out_edge_iterator OutEdgeIterator_t;
				typedef Graph_t::in_edge_iterator InEdgeIterator_t;

				QHash<int, Vertex_t> Package2Vertex_;
				QHash<Dependency, Vertex_t> Dependency2Vertex_;

				typedef QMap<Edge_t, QPair<Vertex_t, Vertex_t> > Edge2Vertices_t;
				Edge2Vertices_t Edge2Vertices_;

				Graph_t Graph_;

				friend struct CycleDetector;
				friend struct FulfillableChecker;
				friend struct VertexPredicate;

				QList<int> PackagesToInstall_;
			public:
				DepTreeBuilder (int);
				virtual ~DepTreeBuilder ();

				/** Whether it is possible to install the package for
				 * which this graph is being built.
				 */
				bool IsFulfilled () const;

				/** Returns the list of the top-level unfulfilled
				 * dependencies.
				 */
				QStringList GetUnfulfilled () const;

				/** @brief Returns the list of packages that need to be
				 * installed in order to install the package passed to
				 * the constructor.
				 *
				 * The return value of this function only makes sense if
				 * the package may be installed at all: if IsFulfilled()
				 * returns true.
				 *
				 * @return The list of packages to be installed.
				 */
				const QList<int>& GetPackagesToInstall () const;
			private:
				/** @brief Builds the part of dependency tree for the
				 * package identified by package.
				 *
				 * Once this function has finished building the layers
				 * of dep tree relevant to the package lpi, it would
				 * recursively call itself on those packages on which
				 * package depends.
				 *
				 * @param[in] package The ID of the package for which
				 * to build the
				 * tree.
				 */
				void InnerLoop (int package);
			};
		};
	};
};

#endif
