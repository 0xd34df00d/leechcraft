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
#include "../../channelsmodel.h"

namespace LC::Aggregator
{
	ItemNavigator::ItemNavigator (QAbstractItemView& view,
			const QModelIndex& selectedChannel,
			const std::function<void (QModelIndex)>& selectChannel)
	: View_ { view }
	, SelectedChannel_ { selectedChannel }
	, SelectChannel_ { selectChannel }
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
		if (SelectPrevUnreadChannel ())
			MoveToPrevUnreadInChannel ();
	}

	void ItemNavigator::MoveToNextUnread () const
	{
		if (MoveToNextUnreadInChannel ())
			return;
		if (SelectNextUnreadChannel ())
			MoveToNextUnreadInChannel ();
	}

	namespace v = std::views;

	bool ItemNavigator::MoveToPrevUnreadInChannel () const
	{
		const auto rc = View_.model ()->rowCount ();
		const auto& current = View_.currentIndex ();
		const auto endRow = current.isValid () ? current.row () : rc;
		return 0 <= endRow - 1 && MoveToUnreadSibling (v::iota (0, endRow - 1) | v::reverse);
	}

	bool ItemNavigator::MoveToNextUnreadInChannel () const
	{
		const auto rc = View_.model ()->rowCount ();
		const auto& current = View_.currentIndex ();
		const auto startRow = current.isValid () ? current.row () + 1 : 0;
		return startRow <= rc - 1 && MoveToUnreadSibling (v::iota (startRow, rc - 1));
	}

	bool ItemNavigator::SelectPrevUnreadChannel() const
	{
		if (!SelectedChannel_.isValid ())
			return false;

		const auto& chanRow = SelectedChannel_.row ();
		const auto& rc = SelectedChannel_.model ()->rowCount ();
		return (0 <= chanRow - 1       && SelectUnreadChannel (v::iota (0, chanRow - 1) | v::reverse)) ||
				(chanRow + 1 <= rc - 1 && SelectUnreadChannel (v::iota (chanRow + 1, rc - 1) | v::reverse));
	}

	bool ItemNavigator::SelectNextUnreadChannel() const
	{
		if (!SelectedChannel_.isValid ())
			return false;

		const auto& chanRow = SelectedChannel_.row ();
		const auto& rc = SelectedChannel_.model ()->rowCount ();
		return (chanRow + 1 <= rc - 1 && SelectUnreadChannel (v::iota (chanRow + 1, rc - 1))) ||
				(0 <= chanRow - 1     && SelectUnreadChannel (v::iota (0, chanRow - 1)));
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

	template<typename Range>
	bool ItemNavigator::SelectUnreadChannel (Range&& rows) const
	{
		for (auto row : rows)
		{
			const auto& otherChannel = SelectedChannel_.sibling (row, ChannelsModel::ColumnUnread);
			if (otherChannel.data ().toInt ())
			{
				SelectChannel_ (otherChannel);
				return true;
			}
		}

		return false;
	}
}
