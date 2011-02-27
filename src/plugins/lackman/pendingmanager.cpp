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

#include "pendingmanager.h"
#include <QStandardItemModel>
#include <QtDebug>
#include "deptreebuilder.h"
#include "core.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LackMan
		{
			PendingManager::PendingManager (QObject *parent)
			: QObject (parent)
			, PendingModel_ (new QStandardItemModel)
			{
			}

			QAbstractItemModel* PendingManager::GetPendingModel () const
			{
				return PendingModel_;
			}

			void PendingManager::Reset ()
			{
				ReinitRootItems ();
				for (int i = AInstall; i < AMAX; ++i)
					ScheduledForAction_ [static_cast<Action> (i)].clear ();
				Deps_.clear ();
				ID2ModelRow_.clear ();
			}

			void PendingManager::ToggleInstallRemove (int id, bool enable, bool installed)
			{
				if (enable)
					EnablePackageInto (id, installed ? ARemove : AInstall);
				else
					DisablePackageFrom (id, installed ? ARemove : AInstall);
			}

			void PendingManager::ToggleUpdate (int id, bool enable)
			{
				if (ScheduledForAction_ [AUpdate].contains (id))
					return;

				if (enable)
					EnablePackageInto (id, AUpdate);
				else
					DisablePackageFrom (id, AUpdate);

				emit packageUpdateToggled (id, enable);
			}

			QSet<int> PendingManager::GetPendingInstall () const
			{
				return ScheduledForAction_ [AInstall];
			}

			QSet<int> PendingManager::GetPendingRemove () const
			{
				return ScheduledForAction_ [ARemove];
			}

			QSet<int> PendingManager::GetPendingUpdate () const
			{
				return ScheduledForAction_ [AUpdate];
			}

			void PendingManager::SuccessfullyInstalled (int packageId)
			{
				DisablePackageFrom (packageId, AInstall);
			}

			void PendingManager::SuccessfullyRemoved (int packageId)
			{
				DisablePackageFrom (packageId, ARemove);
			}

			void PendingManager::SuccessfullyUpdated (int packageId)
			{
				DisablePackageFrom (packageId, AUpdate);
			}

			void PendingManager::EnablePackageInto (int id, PendingManager::Action action)
			{
				DepTreeBuilder builder (id);
				if (!builder.IsFulfilled ())
				{
					QStringList unful = builder.GetUnfulfilled ();
					qWarning () << Q_FUNC_INFO
							<< id
							<< "isn't fulfilled, aborting:"
							<< unful;
					QString list = QString ("<ul><li>%1</li></ul>")
							.arg (unful.join ("</li><li>"));
					throw std::runtime_error (tr ("Package dependencies "
							"could not be fulfilled: %1").arg (list)
							.toUtf8 ().constData ());
				}

				QList<int> deps = builder.GetPackagesToInstall ();
				Deps_ [id] = deps;

				ScheduledForAction_ [action] << id;

				ListPackageInfo info = Core::Instance ().GetListPackageInfo (id);
				QStandardItem *packageItem = new QStandardItem (QString ("%1 (%2)")
						.arg (info.Name_)
						.arg (info.ShortDescription_));
				packageItem->setIcon (Core::Instance ().GetIconForLPI (info));
				Q_FOREACH (int dep, deps)
				{
					info = Core::Instance ().GetListPackageInfo (dep);
					QStandardItem *item = new QStandardItem (QString ("%1 (%2)")
							.arg (info.Name_)
							.arg (info.ShortDescription_));
					packageItem->appendRow (item);
				}

				ID2ModelRow_ [id] = packageItem;
				RootItemForAction_ [action]->appendRow (packageItem);
				
				if (action != ARemove)
					NotifyFetchListUpdate ();
			}

			void PendingManager::DisablePackageFrom (int id, PendingManager::Action action)
			{
				Deps_.remove (id);
				ScheduledForAction_ [action].remove (id);

				if (!ID2ModelRow_.contains (id))
				{
					qWarning () << Q_FUNC_INFO
							<< "strange, seems like"
							<< id
							<< "hasn't been added to the model";
					return;
				}

				QStandardItem *item = ID2ModelRow_.take (id);
				item->parent ()->takeRow (item->row ());
				delete item;
				
				if (action != ARemove)
					NotifyFetchListUpdate ();
			}

			void PendingManager::ReinitRootItems ()
			{
				PendingModel_->clear ();
				for (int i = AInstall; i < AMAX; ++i)
					if (RootItemForAction_.contains (static_cast<Action> (i)))
						delete RootItemForAction_ [static_cast<Action> (i)];

				RootItemForAction_ [AInstall] =
						new QStandardItem (Core::Instance ().GetProxy ()->GetIcon ("addjob"),
								tr ("To be installed"));
				RootItemForAction_ [ARemove] =
						new QStandardItem (Core::Instance ().GetProxy ()->GetIcon ("remove"),
								tr ("To be removed"));
				RootItemForAction_ [AUpdate] =
						new QStandardItem (Core::Instance ().GetProxy ()->GetIcon ("update"),
								tr ("To be updated"));

				for (int i = AInstall; i < AMAX; ++i)
				{
					QStandardItem *item = RootItemForAction_ [static_cast<Action> (i)];
					item->setEditable (false);
					PendingModel_->appendRow (item);
				}
			}
			
			void PendingManager::NotifyFetchListUpdate ()
			{
				QList<int> ids = (ScheduledForAction_ [AInstall] + ScheduledForAction_ [AUpdate]).toList ();
				Q_FOREACH (const int id, ids)
					ids << Deps_ [id];
				emit fetchListUpdated (ids);
			}
		}
	}
}
