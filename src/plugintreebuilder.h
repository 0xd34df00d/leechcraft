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

#ifndef PLUGINTREEBUILDER_H
#define PLUGINTREEBUILDER_H
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/properties.hpp>
#include <QObjectList>
#include <QSet>
#include <QHash>

namespace LeechCraft
{
	class PluginTreeBuilder
	{
		QObjectList Instances_;

		struct VertexInfo
		{
			bool IsFulfilled_;

			/* Needs/provides dependencies: on what features does this
			 * plugin depend.
			 */
			QSet<QString> AllFeatureDeps_;

			/* Second-level plugins dependencies: on what first-level
			 * plugins does this plugin depend.
			 */
			QSet<QByteArray> AllP2PDeps_;

			QSet<QString> UnfulfilledFeatureDeps_;
			QSet<QByteArray> UnfulfilledP2PDeps_;

			QSet<QString> FeatureProvides_;
			QSet<QByteArray> P2PProvides_;

			QObject *Object_;

			VertexInfo ();
			VertexInfo (QObject*);
		};

		typedef boost::property<boost::vertex_color_t, boost::default_color_type,
				VertexInfo> VertexProperty;
		typedef boost::adjacency_list<boost::vecS, boost::vecS,
				boost::bidirectionalS, VertexProperty> Graph_t;
		typedef Graph_t::vertex_descriptor Vertex_t;
		typedef Graph_t::edge_descriptor Edge_t;
		typedef Graph_t::out_edge_iterator OutEdgeIterator_t;
		typedef Graph_t::in_edge_iterator InEdgeIterator_t;

		Graph_t Graph_;

		QHash<QObject*, Vertex_t> Object2Vertex_;
		QObjectList Result_;
	public:
		PluginTreeBuilder ();

		void AddObjects (const QObjectList&);
		void Calculate ();
		QObjectList GetResult () const;
	private:
		void CreateGraph ();
		QMap<Edge_t, QPair<Vertex_t, Vertex_t> > MakeEdges ();
	};
}

#endif
