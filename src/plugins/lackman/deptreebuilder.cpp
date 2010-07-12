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
#include <boost/graph/visitors.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/filtered_graph.hpp>
#include <boost/graph/topological_sort.hpp>
#include "core.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LackMan
		{
			DepTreeBuilder::VertexInfo::VertexInfo ()
			: IsFulfilled_ (false)
			, Type_ (TAny)
			{
			}

			DepTreeBuilder::VertexInfo::VertexInfo (DepTreeBuilder::VertexInfo::Type type)
			: IsFulfilled_ (false)
			, Type_ (type)
			{
			}

			DepTreeBuilder::VertexInfo::VertexInfo (int packageId)
			: IsFulfilled_ (false)
			, Type_ (TAll)
			, PackageId_ (packageId)
			{
			}

			DepTreeBuilder::VertexInfo::VertexInfo (const QString& depName)
			: IsFulfilled_ (false)
			, Type_ (TAny)
			, Dependency_ (depName)
			{
			}

			struct CycleDetector : public boost::default_dfs_visitor
			{
				QList<DepTreeBuilder::Edge_t>& BackEdges_;

				CycleDetector (QList<DepTreeBuilder::Edge_t>& be)
				: BackEdges_ (be)
				{
				}

				template<typename Edge, typename Graph>
				void back_edge (Edge edge, Graph&)
				{
					BackEdges_ << edge;
				}
			};

			struct FulfillableChecker : public boost::default_dfs_visitor
			{
				const QList<DepTreeBuilder::Vertex_t>& BackVertices_;
				const DepTreeBuilder::Edge2Vertices_t E2V_;
				DepTreeBuilder::Graph_t& G_;

				FulfillableChecker (const QList<DepTreeBuilder::Vertex_t>& bv,
						const DepTreeBuilder::Edge2Vertices_t& e2v,
						DepTreeBuilder::Graph_t& g)
				: BackVertices_ (bv)
				, E2V_ (e2v)
				, G_ (g)
				{
				}

				template<typename Vertex, typename Graph>
				void finish_vertex (Vertex u, Graph&)
				{
					if (BackVertices_.contains (u))
					{
						G_ [u].IsFulfilled_ = false;
						return;
					}

					std::pair<DepTreeBuilder::OutEdgeIterator_t,
							DepTreeBuilder::OutEdgeIterator_t> range = boost::out_edges (u, G_);
					if (range.first == range.second)
						G_ [u].IsFulfilled_ = true;
					else
					{
						switch (G_ [u].Type_)
						{
						case DepTreeBuilder::VertexInfo::TAll:
							G_ [u].IsFulfilled_ = true;
							for (DepTreeBuilder::OutEdgeIterator_t i = range.first;
									i < range.second; ++i)
								if (!G_ [GetV (i)].IsFulfilled_)
								{
									G_ [u].IsFulfilled_ = false;
									break;
								}
							break;
						case DepTreeBuilder::VertexInfo::TAny:
							G_ [u].IsFulfilled_ = false;
							for (DepTreeBuilder::OutEdgeIterator_t i = range.first;
									i < range.second; ++i)
								if (G_ [GetV (i)].IsFulfilled_)
								{
									G_ [u].IsFulfilled_ = true;
									break;
								}
							break;
						}
					}
				}

				DepTreeBuilder::Vertex_t GetV (const DepTreeBuilder::OutEdgeIterator_t it)
				{
					return E2V_ [*it].second;
				}
			};

			struct VertexPredicate
			{
				const DepTreeBuilder::Graph_t& G_;
				const DepTreeBuilder::Edge2Vertices_t E2V_;

				VertexPredicate (const DepTreeBuilder::Edge2Vertices_t& e2v,
						const DepTreeBuilder::Graph_t& g)
				: G_ (g)
				{
				}

				template<typename Vertex>
				bool operator() (const Vertex& v) const
				{
					/* If dependency is not fulfilled, we should not
					 * see it in filtered output in any case.
					 */
					if (!G_ [v].IsFulfilled_)
						return false;

					/* If this dependency is of type TAny, then the
					 * parent dependency is of type TAll, and we should
					 * always see it if it's fulfilled (what we've
					 * checked in previous condition).
					 */
					if (G_ [v].Type_ == DepTreeBuilder::VertexInfo::TAny)
						return true;

					/* This dependency is fulfilled, but is of type
					 * TAll. Bad for us: we should step one level up and
					 * check if there is any dependency (which would be
					 * of type TAny) that lists this dependency as first
					 * fulfillable.
					 *
					 * This way we leave only one fulfillable dependency.
					 *
					 * Just as a sidenote, there is little reason in
					 * tying to being "first fulfillable": it'd be much
					 * more sensible to check, for example, if we pull
					 * the least possible amount of additional packages,
					 * but that's too difficult.
					 */
					std::pair<DepTreeBuilder::InEdgeIterator_t,
							DepTreeBuilder::InEdgeIterator_t> range = boost::in_edges (G_, v);
					for (DepTreeBuilder::InEdgeIterator_t i = range.first;
							i < range.second; ++i)
					{
						Vertex u = E2V_ [*i].first;
						std::pair<DepTreeBuilder::OutEdgeIterator_t,
								DepTreeBuilder::OutEdgeIterator_t> sameLevel = boost::out_edges (G_, u);

						for (DepTreeBuilder::OutEdgeIterator_t candIt = sameLevel.first;
								candIt < sameLevel.second; ++candIt)
						{
							Vertex candidate = E2V_ [*candIt].second;
							if (G_ [candidate].IsFulfilled_)
							{
								// If we're here, we're checking the
								// first fulfillable candidate.

								// The next if would succeed only if
								// first fulfillable is the Vertex we
								// are checking.
								if (candidate == v)
									return true;
								else
									break;
							}
						}
					}

					return false;
				}
			};

			DepTreeBuilder::DepTreeBuilder (const ListPackageInfo& packageInfo)
			{
				// First, build the graph.
				Vertex_t root = boost::add_vertex (Graph_);
				Graph_ [root] = VertexInfo (packageInfo.PackageID_);
				Package2Vertex_ [packageInfo.PackageID_] = root;
				InnerLoop (packageInfo);

				// Second, find all the backedges.
				QList<Edge_t> backEdges;
				CycleDetector cd (backEdges);
				boost::depth_first_search (Graph_, boost::visitor (cd));

				// Prepare the list of those vertices that have back
				// edges coming from them.
				QList<Vertex_t> backVertices;
				Q_FOREACH (const Edge_t& edge, backEdges)
					backVertices << Edge2Vertices_ [edge].first;

				// Third, mark fulfillable/unfulfillable deps.
				FulfillableChecker checker (backVertices,
						Edge2Vertices_,
						Graph_);
				boost::depth_first_search (Graph_, boost::visitor (checker));

				// Create filtered graph with only those that are
				// fulfilled.
				boost::filtered_graph<Graph_t,
						boost::keep_all, VertexPredicate> fg (Graph_,
								boost::keep_all (),
								VertexPredicate (Edge2Vertices_, Graph_));

				// Finally run topological sort over filtered graph.
				boost::topological_sort (Graph_,
						std::front_inserter (PackagesToInstall_));
			}

			DepTreeBuilder::~DepTreeBuilder ()
			{
			}

			bool DepTreeBuilder::IsFulfilled () const
			{
			}

			void DepTreeBuilder::InnerLoop (const ListPackageInfo& packageInfo)
			{
				QList<Dependency> dependencies = Core::Instance ().GetDependencies (packageInfo.PackageID_);

				Q_FOREACH (const Dependency& dep, dependencies)
				{
					if (Core::Instance ().IsFulfilled (dep))
						continue;

					Vertex_t depVertex;
					if (!Dependency2Vertex_.contains (dep.Name_))
					{
						depVertex = boost::add_vertex (Graph_);
						Graph_ [depVertex] = VertexInfo (dep.Name_);

						Dependency2Vertex_ [dep.Name_] = depVertex;
					}
					else
						depVertex = Dependency2Vertex_ [dep.Name_];

					Vertex_t packageVertex = Package2Vertex_ [packageInfo.PackageID_];
					Edge_t edge = boost::add_edge (packageVertex, depVertex, Graph_).first;
					Edge2Vertices_ [edge] = qMakePair (packageVertex, depVertex);

					QList<ListPackageInfo> suitable = Core::Instance ().GetDependencyFulfillers (dep);

					Q_FOREACH (const ListPackageInfo& lpi, suitable)
					{
						Vertex_t ffVertex;
						if (!Package2Vertex_.contains (lpi.PackageID_))
						{
							ffVertex = boost::add_vertex (Graph_);
							Graph_ [ffVertex] = VertexInfo (lpi.PackageID_);

							Package2Vertex_ [lpi.PackageID_] = ffVertex;
						}
						else
							ffVertex = Package2Vertex_ [lpi.PackageID_];

						Edge_t edge = boost::add_edge (depVertex, ffVertex, Graph_).first;
						Edge2Vertices_ [edge] = qMakePair (depVertex, ffVertex);
					}
				}
			}
		};
	};
};
