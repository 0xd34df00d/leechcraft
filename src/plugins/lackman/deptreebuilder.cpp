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
			class ExceptionCircularDep : public std::runtime_error
			{
				QStringList DepNames_;
			public:
				ExceptionCircularDep(const QStringList& names)
				: std::runtime_error ("Circular dependency detected.")
				, DepNames_ (names)
				{
				}

				virtual ~ExceptionCircularDep () throw ()
				{
				}

				virtual const char* what () throw ()
				{
					return qPrintable (QString ("Circular dependency detected, backedges to { %1 }").arg (DepNames_.join ("; ")));
				}
			};

			DepTreeBuilder::GraphVertex::GraphVertex (DepTreeBuilder::GraphVertex::Type type)
			: IsFulfilled_ (false)
			, Type_ (type)
			{
			}

			DepTreeBuilder::GraphVertex::GraphVertex (int packageId)
			: IsFulfilled_ (false)
			, Type_ (TAll)
			, PackageId_ (packageId)
			{
			}

			DepTreeBuilder::GraphVertex::GraphVertex (const QString& depName)
			: IsFulfilled_ (false)
			, Type_ (TAny)
			, Dependency_ (depName)
			{
			}

			void DepTreeBuilder::GraphVertex::CheckFulfilled ()
			{
				switch (Type_)
				{
				case TAll:
					IsFulfilled_ = true;
					Q_FOREACH (const GraphVertex_ptr& depVertex,
							ChildVertices_)
						if (!depVertex->IsFulfilled_)
						{
							IsFulfilled_ = false;
							break;
						}
					break;
				case TAny:
					IsFulfilled_ = false;
					Q_FOREACH (const GraphVertex_ptr& packageVertex,
							ChildVertices_)
						if (packageVertex->IsFulfilled_)
						{
							IsFulfilled_ = true;
							break;
						}
					break;
				}
			}

			DepTreeBuilder::DepTreeBuilder ()
			{
			}

			DepTreeBuilder::~DepTreeBuilder ()
			{
			}

			void DepTreeBuilder::BuildFor (const ListPackageInfo& packageInfo)
			{
				GraphRoot_.reset (new GraphVertex (GraphVertex::TAll));
				BuildInnerLoop (packageInfo, GraphRoot_);
				GraphRoot_->CheckFulfilled ();
			}

			bool DepTreeBuilder::IsFulfilled () const
			{
				return GraphRoot_->IsFulfilled_;
			}

			namespace
			{
				struct StackGuard
				{
					DepTreeBuilder::DFSLevels_t& Stack_;

					StackGuard (const QString& packageName,
							DepTreeBuilder::DFSLevels_t& stack)
					: Stack_ (stack)
					{
						Stack_.push (packageName);
					}

					~StackGuard ()
					{
						Stack_.pop ();
					}
				};
			}

			void DepTreeBuilder::BuildInnerLoop (const ListPackageInfo& packageInfo,
					DepTreeBuilder::GraphVertex_ptr parentVertex)
			{
				StackGuard sg (packageInfo.Name_, Levels_);

				QList<Dependency> dependencies = Core::Instance ().GetDependencies (packageInfo.PackageID_);

				Q_FOREACH (const Dependency& dep, dependencies)
				{
					if (Core::Instance ().IsFulfilled (dep))
						continue;

					GraphVertex_ptr depVertex (new GraphVertex (dep.Name_));
					parentVertex->ChildVertices_ << depVertex;

					QList<ListPackageInfo> suitable = Core::Instance ().GetDependencyFulfillers (dep);

					QStringList failedDeps;
					Q_FOREACH (const ListPackageInfo& lpi, suitable)
						if (Levels_.contains (lpi.Name_))
						{
							failedDeps << lpi.Name_;
							suitable.removeAll (lpi);
						}

					if (!suitable.size ())
						continue;

					Q_FOREACH (const ListPackageInfo& lpi, suitable)
					{
						GraphVertex_ptr packageVertex (new GraphVertex (lpi.PackageID_));
						depVertex->ChildVertices_ << packageVertex;

						BuildInnerLoop (lpi, packageVertex);
					}

					depVertex->CheckFulfilled ();
				}

				parentVertex->CheckFulfilled ();
			}
		};
	};
};
