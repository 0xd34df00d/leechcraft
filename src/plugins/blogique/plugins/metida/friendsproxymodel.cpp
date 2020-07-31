/**********************************************************************
 *  LeechCraft - modular cross-platform feature rich internet client.
 *  Copyright (C) 2010-2013  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "friendsproxymodel.h"

namespace LC
{
namespace Blogique
{
namespace Metida
{
	FriendsProxyModel::FriendsProxyModel (QObject *parent)
	: QSortFilterProxyModel (parent)
	{
		setFilterCaseSensitivity (Qt::CaseInsensitive);
		setSortCaseSensitivity (Qt::CaseInsensitive);
	}

	bool FriendsProxyModel::filterAcceptsRow (int sourceRow, const QModelIndex& sourceParent) const
	{
		if (filterRegExp ().isEmpty ())
			return true;
		const auto& nick = sourceModel ()->index (sourceRow, CNickname, sourceParent).data ().toString ();
		const auto& name = sourceModel ()->index (sourceRow, CUsername, sourceParent).data ().toString ();
		const auto& birthday = sourceModel ()->index (sourceRow, CBirthday, sourceParent).data ().toString ();
		return nick.contains (filterRegExp ()) || name.contains (filterRegExp ()) || birthday.contains (filterRegExp ());
	}

	bool FriendsProxyModel::lessThan (const QModelIndex& left, const QModelIndex& right) const
	{
		if (left.column () != CFriendStatus || right.column () != CFriendStatus)
			return QSortFilterProxyModel::lessThan (left, right);
		
		const int leftStatus = sourceModel ()->data (left, FRFriendStatus).toInt ();
		const int rightStatus = sourceModel ()->data (right, FRFriendStatus).toInt ();
		switch (leftStatus)
		{
		case FSFriendOf:
			return true;
		case FSMyFriend:
			switch (rightStatus)
			{
			case FSFriendOf:
				return false;
			case FSMyFriend:
				return true;
			case FSBothFriends:
				return true;
			default:
				return true;
			}
		case FSBothFriends:
			return false;
		default:
			return true;
		}
		return true;
	}

}
}
}
