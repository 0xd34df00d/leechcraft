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

#include "queueiteminfo.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace DCminator
		{
			QueueItemInfo::QueueItemInfo (const dcpp::QueueItem& aqi)
			: dcpp::Flags (aqi)
			, Target_ (aqi.getTarget ())
			, Path_ (dcpp::Util::getFilePath (aqi.getTarget ()))
			, Size_ (aqi.getSize ())
			, DownloadedBytes_ (aqi.getDownloadedBytes ())
			, Added_ (aqi.getAdded ())
			, Priority_ (aqi.getPriority ())
			, Running_ (false)
			, TTH_ (aqi.getTTH ())
			, Sources_ (aqi.getSources ())
			, BadSources_ (aqi.getBadSources ())
			, UpdateMask_ (-1)
			{
			}

			QueueItemInfo::~QueueItemInfo ()
			{
			}

			std::string QueueItemInfo::GetTarget () const
			{
				return Target_;
			}

			std::string QueueItemInfo::GetPath () const
			{
				return Path_;
			}

			int64_t QueueItemInfo::GetSize () const
			{
				return Size_;
			}

			int64_t QueueItemInfo::GetDownloadedBytes () const
			{
				return DownloadedBytes_;
			}

			time_t QueueItemInfo::GetAdded () const
			{
				return Added_;
			}

			void QueueItemInfo::SetPriority (const dcpp::QueueItem::Priority& prio)
			{
				Priority_ = prio;
			}

			void QueueItemInfo::SetDownloadedBytes (int64_t db)
			{
				DownloadedBytes_ = db;
			}

			void QueueItemInfo::SetSources (const dcpp::QueueItem::SourceList& s)
			{
				Sources_ = s;
			}

			void QueueItemInfo::SetBadSources (const dcpp::QueueItem::SourceList& s)
			{
				BadSources_ = s;
			}
		};
	};
};

