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

#ifndef PLUGINS_AGGREGATOR_INTERFACES_AGGREGATOR_IPROXYOBJECT_H
#define PLUGINS_AGGREGATOR_INTERFACES_AGGREGATOR_IPROXYOBJECT_H
#include <boost/shared_ptr.hpp>

namespace LeechCraft
{
namespace Aggregator
{
	struct Item;
	struct Channel;
	struct Feed;

	typedef boost::shared_ptr<Item> Item_ptr;
	typedef boost::shared_ptr<Channel> Channel_ptr;
	typedef boost::shared_ptr<Feed> Feed_ptr;

	class IProxyObject
	{
	public:
		virtual ~IProxyObject () {}

		virtual void AddFeed (Feed_ptr) = 0;
		virtual void AddChannel (Channel_ptr) = 0;
		virtual void AddItem (Item_ptr) = 0;
	};

	typedef boost::shared_ptr<IProxyObject> IProxyObject_ptr;
}
}

Q_DECLARE_INTERFACE (LeechCraft::Aggregator::IProxyObject,
		"org.Deviant.LeechCraft.Aggregator.IProxyObject/1.0");

#endif
