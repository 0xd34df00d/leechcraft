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

#pragma once

#include <Wt/WApplication>
#include <Wt/WModelIndex>
#include <interfaces/core/icoreproxy.h>

namespace LeechCraft
{
namespace Aggregator
{
class IProxyObject;

namespace WebAccess
{
	class ReadChannelsFilter;

	class AggregatorApp : public Wt::WApplication
	{
		IProxyObject *AP_;
		ICoreProxy_ptr CP_;

		Wt::WStandardItemModel *ChannelsModel_;
		ReadChannelsFilter *ChannelsFilter_;
		Wt::WStandardItemModel *ItemsModel_;

		Wt::WTableView *ItemsTable_;

		Wt::WText *ItemView_;
	public:
		enum ChannelRole
		{
			CID = Wt::UserRole + 1,
			FID,
			UnreadCount
		};

		enum ItemRole
		{
			IID = Wt::UserRole + 1,
			ParentCh,
			IsUnread,
			Text,
			Link
		};

		AggregatorApp (IProxyObject*, ICoreProxy_ptr, const Wt::WEnvironment& environment);
	private:
		void HandleChannelClicked (const Wt::WModelIndex&);
		void HandleItemClicked (const Wt::WModelIndex&);
		void SetupUI ();
	};
}
}
}
