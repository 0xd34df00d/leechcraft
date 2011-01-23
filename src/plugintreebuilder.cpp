/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#include "plugintreebuilder.h"
#include <boost/graph/visitors.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/filtered_graph.hpp>
#include <boost/graph/topological_sort.hpp>
#include "interfaces/iinfo.h"
#include "interfaces/iplugin2.h"
#include "interfaces/ipluginready.h"

namespace LeechCraft
{
	PluginTreeBuilder::VertexInfo::VertexInfo ()
	: IsFulfilled_ (false)
	, Object_ (0)
	{
	}

	PluginTreeBuilder::VertexInfo::VertexInfo (QObject *obj)
	: IsFulfilled_ (false)
	, Object_ (obj)
	{
		IInfo *info = qobject_cast<IInfo*> (obj);
		if (!info)
		{
			qWarning () << Q_FUNC_INFO
					<< obj
					<< "doesn't implement IInfo";
			throw std::runtime_error ("VertexInfo creation failed.");
		}

		AllFeatureDeps_ = QSet<QString>::fromList (info->Uses ());
		UnfulfilledFeatureDeps_ = AllFeatureDeps_;
		FeatureProvides_ = QSet<QString>::fromList (info->Provides ());

		IPluginReady *ipr = qobject_cast<IPluginReady*> (obj);
		if (ipr)
			P2PProvides_ = ipr->GetExpectedPluginClasses ();

		IPlugin2 *ip2 = qobject_cast<IPlugin2*> (obj);
		if (ip2)
		{
			AllP2PDeps_ = ip2->GetPluginClasses ();
			UnfulfilledP2PDeps_ = AllP2PDeps_;
		}
	}

	PluginTreeBuilder::PluginTreeBuilder ()
	{
	}

	void PluginTreeBuilder::AddObjects (const QObjectList& objs)
	{
		Instances_ << objs;
	}

	template<typename Edge>
	struct CycleDetector : public boost::default_dfs_visitor
	{
		QList<Edge>& BackEdges_;

		CycleDetector (QList<Edge>& be)
		: BackEdges_ (be)
		{
		}

		template<typename Graph>
		void back_edge (Edge edge, Graph&)
		{
			BackEdges_ << edge;
		}
	};

	template<typename Graph, typename Vertex>
	struct FulfillableChecker : public boost::default_dfs_visitor
	{
		Graph& G_;
		const QList<Vertex>& BackVerts_;
		const QMap<Vertex, QList<Vertex> >& Reachable_;

		FulfillableChecker (Graph& g,
				const QList<Vertex>& backVerts,
				const QMap<Vertex, QList<Vertex> >& reachable)
		: G_ (g)
		, BackVerts_ (backVerts)
		, Reachable_ (reachable)
		{
		}

		void finish_vertex (Vertex u, const Graph& g)
		{
			if (BackVerts_.contains (u))
			{
				qWarning () << G_ [u].Object_ << "is backedge";
				return;
			}

			G_ [u].IsFulfilled_ = true;

			Q_FOREACH (const Vertex& v, Reachable_ [u])
				if (!G_ [v].IsFulfilled_)
				{
					G_ [u].IsFulfilled_ = false;
					break;
				}
		}
	};

	template<typename Graph>
	struct VertexPredicate
	{
		Graph *G_;

		VertexPredicate ()
		: G_ (0)
		{
		}

		VertexPredicate (Graph& g)
		: G_ (&g)
		{
		}

		template<typename Vertex>
		bool operator() (const Vertex& u) const
		{
			return (*G_) [u].IsFulfilled_;
		}
	};

	void PluginTreeBuilder::Calculate ()
	{
		Graph_.clear ();
		Result_.clear ();

		CreateGraph ();
		QMap<Edge_t, QPair<Vertex_t, Vertex_t> > edge2vert = MakeEdges ();

		QMap<Vertex_t, QList<Vertex_t> > reachable;
		QPair<Vertex_t, Vertex_t> pair;
		Q_FOREACH (pair, edge2vert)
			reachable [pair.first] << pair.second;

		QList<Edge_t> backEdges;
		CycleDetector<Edge_t> cd (backEdges);
		boost::depth_first_search (Graph_, boost::visitor (cd));

		QList<Vertex_t> backVertices;
		Q_FOREACH (const Edge_t& backEdge, backEdges)
			backVertices << edge2vert [backEdge].first;

		FulfillableChecker<Graph_t, Vertex_t> checker (Graph_, backVertices, reachable);
		boost::depth_first_search (Graph_, boost::visitor (checker));

		typedef boost::filtered_graph<Graph_t, boost::keep_all, VertexPredicate<Graph_t> > fg_t;
		fg_t fg = fg_t (Graph_,
				boost::keep_all (),
				VertexPredicate<Graph_t> (Graph_));

		QList<Vertex_t> vertices;
		boost::topological_sort (fg,
				std::back_inserter (vertices));
		Q_FOREACH (const Vertex_t& vertex, vertices)
			Result_ << fg [vertex].Object_;
	}

	QObjectList PluginTreeBuilder::GetResult () const
	{
		return Result_;
	}

	void PluginTreeBuilder::CreateGraph ()
	{
		Q_FOREACH (QObject *object, Instances_)
		{
			try
			{
				VertexInfo vi (object);
				Vertex_t objVertex = boost::add_vertex (Graph_);
				Graph_ [objVertex] = vi;
				Object2Vertex_ [object] = objVertex;
			}
			catch (const std::exception& e)
			{
				qWarning () << Q_FUNC_INFO
						<< e.what ()
						<< "for"
						<< object
						<< "; skipping";
				continue;
			}
		}
	}

	QMap<PluginTreeBuilder::Edge_t, QPair<PluginTreeBuilder::Vertex_t, PluginTreeBuilder::Vertex_t> > PluginTreeBuilder::MakeEdges ()
	{
		QSet<QPair<Vertex_t, Vertex_t> > depVertices;
		boost::graph_traits<Graph_t>::vertex_iterator vi, vi_end;
		for (boost::tie (vi, vi_end) = boost::vertices (Graph_); vi != vi_end; ++vi)
		{
			const QSet<QString>& fdeps = Graph_ [*vi].UnfulfilledFeatureDeps_;
			const QSet<QByteArray>& pdeps = Graph_ [*vi].UnfulfilledP2PDeps_;
			if (!fdeps.size () && !pdeps.size ())
				continue;

			boost::graph_traits<Graph_t>::vertex_iterator vj, vj_end;
			for (boost::tie (vj, vj_end) = boost::vertices (Graph_); vj != vj_end; ++vj)
			{
				const QSet<QString>& fInter = Graph_ [*vj].FeatureProvides_.intersect (fdeps);
				if (fInter.size ())
				{
					Graph_ [*vi].UnfulfilledFeatureDeps_.subtract (fInter);
					depVertices << qMakePair (*vi, *vj);
				}

				const QSet<QByteArray>& pInter = Graph_ [*vj].P2PProvides_.intersect (pdeps);
				if (pInter.size ())
				{
					Graph_ [*vi].UnfulfilledP2PDeps_.subtract (pInter);
					depVertices << qMakePair (*vi, *vj);
				}
			}
		}

		QMap<Edge_t, QPair<Vertex_t, Vertex_t> > result;
		QPair<Vertex_t, Vertex_t> pair;
		Q_FOREACH (pair, depVertices)
			result [boost::add_edge (pair.first, pair.second, Graph_).first] = pair;
		return result;
	}
}
