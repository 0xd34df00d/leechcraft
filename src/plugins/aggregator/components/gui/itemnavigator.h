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
	class ItemNavigator
	{
		QAbstractItemView& View_;
		const QModelIndex& SelectedChannel_;
		const std::function<void (QModelIndex)> SelectChannel_;
	public:
		explicit ItemNavigator (QAbstractItemView&,
				const QModelIndex&,
				const std::function<void (QModelIndex)>&);

		void MoveToPrev () const;
		void MoveToNext () const;

		void MoveToPrevUnread () const;
		void MoveToNextUnread () const;
	private:
		void MoveToSibling (int delta) const;

		bool MoveToPrevUnreadInChannel () const;
		bool MoveToNextUnreadInChannel () const;

		bool SelectPrevUnreadChannel () const;
		bool SelectNextUnreadChannel () const;

		template<typename Range>
		bool MoveToUnreadSibling (Range&& r) const;

		template<typename Range>
		bool SelectUnreadChannel (Range&& r) const;
	};
}
