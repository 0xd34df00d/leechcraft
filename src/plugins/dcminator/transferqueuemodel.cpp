/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2009  Georg Rudoy
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

#include "transferqueuemodel.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace DCminator
		{
			TransferQueueModel::TransferQueueModel (QObject *parent)
			: QAbstractItemModel (parent)
			{
				AddQueueList (dcpp::QueueManager::getInstance ()->lockQueue ());
				dcpp::QueueManager::getInstance ()->unlockQueue ();
				dcpp::QueueManager::getInstance ()->addListener (this);
			}

			void TransferQueueModel::AddQueueList (const dcpp::QueueItem::StringMap& li)
			{
				for (dcpp::QueueItem::StringMap::const_iterator i = li.begin (),
						end = li.end (); i != end; ++i)
					AddQueueItem (QueueItemInfo_ptr (new QueueItemInfo (*i->second)));
			}
			
			void TransferQueueModel::AddQueueItem (const QueueItemInfo_ptr& qi)
			{
				DirectoryMap_.insert (dcpp::make_pair (qi->GetPath (), qi));
				beginInsertRows (QModelIndex (), Items_.size (), Items_.size ());
				Items_ << qi;
				endInsertRows ();
			}

			void TransferQueueModel::RemoveQueueItem (const std::string&)
			{
				// TODO implement
			}

			void TransferQueueModel::on (dcpp::QueueManagerListener::Added, dcpp::QueueItem* qi) throw ()
			{
				AddQueueItem (QueueItemInfo_ptr (new QueueItemInfo (*qi)));
			}

			void TransferQueueModel::on (dcpp::QueueManagerListener::Moved,
					dcpp::QueueItem* qi, const std::string& old) throw ()
			{
				RemoveQueueItem (old);
				AddQueueItem (QueueItemInfo_ptr (new QueueItemInfo (*qi)));
			}
		};
	};
};
