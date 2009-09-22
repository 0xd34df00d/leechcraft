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

#ifndef PLUGINS_DCMINATOR_QUEUEITEMINFO_H
#define PLUGINS_DCMINATOR_QUEUEITEMINFO_H
#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/QueueManager.h"
#include "dcpp/FastAlloc.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace DCminator
		{
			class QueueItemInfo : public dcpp::Flags
								 , public dcpp::FastAlloc<QueueItemInfo>
			{
				std::string Target_;
				std::string Path_;
				int64_t Size_;
				int64_t DownloadedBytes_;
				time_t Added_;
				dcpp::QueueItem::Priority Priority_;
				bool Running_;
				dcpp::TTHValue TTH_;
				dcpp::QueueItem::SourceList Sources_;
				dcpp::QueueItem::SourceList BadSources_;
				uint32_t UpdateMask_;
			public:
				QueueItemInfo (const dcpp::QueueItem&);
				virtual ~QueueItemInfo ();

				std::string GetTarget () const;
				std::string GetPath () const;
				int64_t GetSize () const;
				int64_t GetDownloadedBytes () const;
				time_t GetAdded () const;

				void SetPriority (const dcpp::QueueItem::Priority&);
				void SetDownloadedBytes (int64_t);
				void SetSources (const dcpp::QueueItem::SourceList&);
				void SetBadSources (const dcpp::QueueItem::SourceList&);
			};

			typedef boost::shared_ptr<QueueItemInfo> QueueItemInfo_ptr;
		};
	};
};

#endif

