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
#include <QtDebug>

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

			TransferQueueModel::~TransferQueueModel ()
			{
				dcpp::QueueManager::getInstance ()->removeListener (this);
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

			QueueItemInfo_ptr TransferQueueModel::GetItemInfo (const std::string& target)
			{
				std::string path = dcpp::Util::getFilePath (target);
				DirectoryPair_t items = DirectoryMap_.equal_range (path);
				for (DirectoryIter_t i = items.first; i != items.second; ++i)
					if (i->second->GetTarget () == target)
						return i->second;

				return QueueItemInfo_ptr ();
			}

			void TransferQueueModel::UpdateQueueItem (const std::string& param,
					const dcpp::QueueItem& item)
			{
				QueueItemInfo_ptr ii = GetItemInfo (param);
				if (!ii)
				{
					qWarning () << Q_FUNC_INFO
						<< "unable to get QueueItemInfo_ptr for"
						<< param.c_str ();
					return;
				}

				ii->SetPriority (item.getPriority ());
				ii->SetDownloadedBytes (item.getDownloadedBytes ());
				ii->SetSources (item.getSources ());
				ii->SetBadSources (item.getBadSources ());

				int pos = Items_.indexOf (ii);
				if (pos == -1)
				{
					qWarning () << Q_FUNC_INFO
						<< "not found element in the Items_"
						<< param.c_str ();
					return;
				}

				emit dataChanged (index (pos, 0), index (pos, columnCount () - 1));
			}

			void TransferQueueModel::RemoveQueueItem (const std::string& param)
			{
				QueueItemInfo_ptr ii = GetItemInfo (param);
				if (!ii)
				{
					qWarning () << Q_FUNC_INFO
						<< "unable to get QueueItemInfo_ptr for"
						<< param.c_str ();
					return;
				}

				DirectoryPair_t i = DirectoryMap_.equal_range (ii->GetPath ());
				DirectoryIter_t j;
				for (j = i.first; j != i.second; ++j)
					if (j->second == ii)
						break;

				if (j == i.second)
				{
					qWarning () << Q_FUNC_INFO
						<< "not found element for string"
						<< param.c_str ();
					return;
				}
				DirectoryMap_.erase (j);

				int pos = Items_.indexOf (ii);
				if (pos == -1)
				{
					qWarning () << Q_FUNC_INFO
						<< "not found element in the Items_"
						<< param.c_str ();
					return;
				}
				beginRemoveRows (QModelIndex (), pos, pos);
				Items_.removeAt (pos);
				endRemoveRows ();
			}

			void TransferQueueModel::on (dcpp::QueueManagerListener::Added, dcpp::QueueItem* qi) throw ()
			{
				AddQueueItem (QueueItemInfo_ptr (new QueueItemInfo (*qi)));
			}

			void TransferQueueModel::on (dcpp::QueueManagerListener::Moved,
					dcpp::QueueItem *qi, const std::string& old) throw ()
			{
				RemoveQueueItem (old);
				AddQueueItem (QueueItemInfo_ptr (new QueueItemInfo (*qi)));
			}

			void TransferQueueModel::on (dcpp::QueueManagerListener::Removed,
					dcpp::QueueItem *qi) throw ()
			{
				RemoveQueueItem (qi->getTarget ());
			}

			void TransferQueueModel::on (dcpp::QueueManagerListener::SourcesUpdated,
					dcpp::QueueItem *qi) throw ()
			{
				UpdateQueueItem (qi->getTarget (), *qi);
			}

			void TransferQueueModel::on (dcpp::QueueManagerListener::StatusUpdated,
					dcpp::QueueItem *qi) throw ()
			{
				on (dcpp::QueueManagerListener::SourcesUpdated (), qi);
			}
		};
	};
};
