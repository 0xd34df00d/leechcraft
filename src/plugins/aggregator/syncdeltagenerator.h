/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#ifndef PLUGINS_AGGREGATOR_SYNCDELTAGENERATOR_H
#define PLUGINS_AGGREGATOR_SYNCDELTAGENERATOR_H
#include <interfaces/isyncable.h>
#include <util/syncops.h>
#include "channel.h"
#include "item.h"

namespace LeechCraft
{
namespace Aggregator
{
	struct Feed;

	class SyncDeltaGenerator
	{
		enum PayloadType
		{
			PTFeedAdded = 1,
			PTChanAdded,
			PTItemAdded,
			PTItemRead,
			PTChannelTagsChanged,
			PTItemTagsChanged
		};
	public:
		Sync::Payloads_t GetFeedAdded (const Feed&);
		Sync::Payloads_t GetChanAdded (Channel_ptr);
		Sync::Payloads_t GetItemAdded (Item_ptr);
		Sync::Payloads_t GetItemRead (Item_ptr, bool);
		Sync::Payloads_t GetChannelTagsChanged (Channel_ptr, const QStringList&);
		Sync::Payloads_t GetItemTagsChanged (Item_ptr, const QStringList&);
		
		void ParseDelta (const Sync::Payload&);
	};
}
}

#endif
