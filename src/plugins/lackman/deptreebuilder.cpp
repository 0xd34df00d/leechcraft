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

			DepTreeBuilder::DepTreeBuilder ()
			{
			}

			DepTreeBuilder::~DepTreeBuilder ()
			{
			}

			void DepTreeBuilder::BuildFor (const ListPackageInfo& packageInfo)
			{
				GraphRoot_.reset (new GraphNode (GraphNode::TAll));
				BuildInnerLoop (packageInfo);
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

			void DepTreeBuilder::BuildInnerLoop (const ListPackageInfo& packageInfo)
			{
				StackGuard sg (packageInfo.Name_, Levels_);

				QList<Dependency> dependencies = Core::Instance ().GetDependencies (packageInfo.PackageID_);

				Q_FOREACH (const Dependency& dep, dependencies)
				{
					if (Core::Instance ().IsFulfilled (dep))
						continue;

					QList<ListPackageInfo> suitable = Core::Instance ().GetDependencyFulfillers (dep);

					QStringList failedDeps;
					Q_FOREACH (const ListPackageInfo& lpi, suitable)
						if (Levels_.contains (lpi.Name_))
						{
							failedDeps << lpi.Name_;
							suitable.removeAll (lpi);
						}

					if (!suitable.size ())
						throw ExceptionCircularDep (failedDeps);
				}
			}
		};
	};
};
