/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "itemnavigator.h"
#include <ranges>
#include <QAbstractItemView>
#include "interfaces/aggregator/iitemsmodel.h"
#include "../models/channelsmodel.h"

namespace LC::Aggregator
{
	ItemNavigator::ItemNavigator (QAbstractItemView& view, std::function<bool (ChannelDirection)> selectChannel)
	: View_ { view }
	, SelectChannel_ { std::move (selectChannel) }
	{
	}

	void ItemNavigator::MoveToPrev () const
	{
		MoveToSibling (-1);
	}

	void ItemNavigator::MoveToNext () const
	{
		MoveToSibling (+1);
	}

	void ItemNavigator::MoveToSibling (int delta) const
	{
		const auto& current = View_.currentIndex ();
		const auto& target = current.siblingAtRow (current.row () + delta);
		if (target.isValid ())
			View_.setCurrentIndex (target);
	}

	void ItemNavigator::MoveToPrevUnread () const
	{
		if (MoveToPrevUnreadInChannel ())
			return;
		if (SelectChannel_ (ChannelDirection::PreviousUnread))
			MoveToPrevUnreadInChannel ();
	}

	void ItemNavigator::MoveToNextUnread () const
	{
		if (MoveToNextUnreadInChannel ())
			return;
		if (SelectChannel_ (ChannelDirection::NextUnread))
			MoveToNextUnreadInChannel ();
	}

	namespace v = std::views;

	bool ItemNavigator::MoveToPrevUnreadInChannel () const
	{
		const auto rc = View_.model ()->rowCount ();
		const auto& current = View_.currentIndex ();
		const auto endRow = current.isValid () ? current.row () : rc;
		return 0 <= endRow && MoveToUnreadSibling (v::iota (0, endRow) | v::reverse);
	}

	bool ItemNavigator::MoveToNextUnreadInChannel () const
	{
		const auto rc = View_.model ()->rowCount ();
		const auto& current = View_.currentIndex ();
		const auto startRow = current.isValid () ? current.row () + 1 : 0;
		return startRow <= rc && MoveToUnreadSibling (v::iota (startRow, rc));
	}

	template<typename Range>
	bool ItemNavigator::MoveToUnreadSibling (Range&& rows) const
	{
		const auto& model = *View_.model ();
		for (auto row : rows)
		{
			const auto index = model.index (row, 0);
			if (!index.data (IItemsModel::ItemRole::IsRead).toBool ())
			{
				View_.setCurrentIndex (index);
				return true;
			}
		}

		return false;
	}
}
