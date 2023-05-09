/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>

class QAbstractItemView;
class QModelIndex;

namespace LC::Aggregator
{
	enum class ChannelDirection;

	class ItemNavigator
	{
		QAbstractItemView& View_;
		const std::function<bool (ChannelDirection)> SelectChannel_;
	public:
		explicit ItemNavigator (QAbstractItemView& view, std::function<bool (ChannelDirection)>);

		void MoveToPrev () const;
		void MoveToNext () const;

		void MoveToPrevUnread () const;
		void MoveToNextUnread () const;
	private:
		void MoveToSibling (int delta) const;

		bool MoveToPrevUnreadInChannel () const;
		bool MoveToNextUnreadInChannel () const;

		template<typename Range>
		bool MoveToUnreadSibling (Range&& r) const;
	};
}
