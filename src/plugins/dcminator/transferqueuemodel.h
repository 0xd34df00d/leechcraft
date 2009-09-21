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

#ifndef PLUGINS_DCMINATOR_TRANSFERQUEUEMODEL_H
#define PLUGINS_DCMINATOR_TRANSFERQUEUEMODEL_H
#include <boost/shared_ptr.hpp>
#include <QAbstractItemModel>
#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/QueueManager.h"
#include "queueiteminfo.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace DCminator
		{
			class TransferQueueModel : public QAbstractItemModel
									 , private dcpp::QueueManagerListener
			{
				Q_OBJECT

				typedef boost::unordered_multimap<std::string,
						QueueItemInfo_ptr,
						dcpp::noCaseStringHash,
						dcpp::noCaseStringEq> DirectoryMap_t;
				typedef DirectoryMap_t::iterator DirectoryIter_t;
				typedef std::pair<DirectoryIter_t, DirectoryIter_t> DirectoryPair_t;
				DirectoryMap_t DirectoryMap_;

				QList<QueueItemInfo_ptr> Items_;
			public:
				TransferQueueModel (QObject* = 0);
				virtual ~TransferQueueModel ();
			private:
				void AddQueueList (const dcpp::QueueItem::StringMap&);
				void AddQueueItem (const QueueItemInfo_ptr&);
				QueueItemInfo_ptr GetItemInfo (const std::string&);
				void UpdateQueueItem (const std::string&, const dcpp::QueueItem&);
				void RemoveQueueItem (const std::string&);

				virtual void on (dcpp::QueueManagerListener::Added,
						dcpp::QueueItem*) throw ();
				virtual void on (dcpp::QueueManagerListener::Moved,
						dcpp::QueueItem*,
						const std::string&) throw ();
				virtual void on (dcpp::QueueManagerListener::Removed,
						dcpp::QueueItem*) throw ();
				virtual void on (dcpp::QueueManagerListener::SourcesUpdated, 
						dcpp::QueueItem*) throw ();
				virtual void on (dcpp::QueueManagerListener::StatusUpdated,
						dcpp::QueueItem*) throw ();
			};
		};
	};
};

#endif

