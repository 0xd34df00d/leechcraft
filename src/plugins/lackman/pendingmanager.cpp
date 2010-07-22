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
				PendingModel_->clear ();
				ScheduledInstall_.clear ();
				ScheduledRemove_.clear ();
				ScheduledUpdate_.clear ();
			}

			void PendingManager::ToggleInstallRemove (int id, bool enable, bool installed)
			{
				if (enable)
					EnablePackageInto (id, installed ? ScheduledRemove_ : ScheduledInstall_);
				else
					DisablePackageFrom (id, installed ? ScheduledRemove_ : ScheduledInstall_);
			}

			void PendingManager::ToggleUpdate (int id, bool enable)
			{
				if (enable)
					EnablePackageInto (id, ScheduledUpdate_);
				else
					DisablePackageFrom (id, ScheduledUpdate_);
			}

			void PendingManager::EnablePackageInto (int id, QSet<int>& container)
			{
				DepTreeBuilder builder (id);
				if (!builder.IsFulfilled ())
				{
					qWarning () << Q_FUNC_INFO
							<< id
							<< "isn't fulfilled, aborting";
					throw std::runtime_error (QString ("Package dependencies "
							"could not be fulfilled.").toUtf8 ().constData ());
				}

				QList<int> deps = builder.GetPackagesToInstall ();
				Deps_ [id] = deps;

				container << id;

				ListPackageInfo info = Core::Instance ().GetListPackageInfo (id);
				QStandardItem *packageItem = new QStandardItem (QString ("%1 (%2)")
						.arg (info.Name_)
						.arg (info.ShortDescription_));
				Q_FOREACH (int dep, deps)
				{
					info = Core::Instance ().GetListPackageInfo (id);
					QStandardItem *item = new QStandardItem (QString ("%1 (%2)")
							.arg (info.Name_)
							.arg (info.ShortDescription_));
					packageItem->appendRow (item);
				}

				ID2ModelRow_ [id] = packageItem;
				PendingModel_->appendRow (packageItem);
			}

			void PendingManager::DisablePackageFrom (int id, QSet<int>& container)
			{
				Deps_.remove (id);
				container.remove (id);

				delete ID2ModelRow_.take (id);
			}
		}
	}
}
