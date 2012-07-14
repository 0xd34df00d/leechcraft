/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#include "tovarmaps.h"

namespace LeechCraft
{
namespace Aggregator
{
	QVariantMap GetItemMapChannelPart (const Channel_ptr channel)
	{
		QVariantMap result;
		result ["ChannelID"] = channel->ChannelID_;
		result ["ChannelTitle"] = channel->Title_;
		result ["ChannelLink"] = channel->Link_;
		result ["ChannelTags"] = channel->Tags_;
		return result;
	}

	QVariantMap GetItemMapItemPart (const Item_ptr item)
	{
		QVariantMap result;
		result ["ItemID"] = item->ItemID_;
		result ["ItemTitle"] = item->Title_;
		result ["ItemLink"] = item->Link_;
		result ["ItemDescription"] = item->Description_;
		result ["ItemCategories"] = item->Categories_;
		result ["ItemPubdate"] = item->PubDate_;
		result ["ItemCommentsLink"] = item->CommentsLink_;
		result ["ItemCommentsPageLink"] = item->CommentsPageLink_;
		return result;
	}

	QVariantList GetItems (const Channel_ptr channel)
	{
		QVariantList result;
		const QVariantMap& channelPart = GetItemMapChannelPart (channel);
		Q_FOREACH (const Item_ptr item, channel->Items_)
			result << GetItemMapItemPart (item).unite (channelPart);
		return result;
	}
}
}
