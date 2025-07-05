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
	: FixedStringFilterProxyModel (parent)
	{
		setSortCaseSensitivity (Qt::CaseInsensitive);
		SetFilterColumns ({ CNickname, CUsername, CBirthday });
	}

	bool FriendsProxyModel::lessThan (const QModelIndex& left, const QModelIndex& right) const
	{
		if (left.column () != CFriendStatus || right.column () != CFriendStatus)
			return FixedStringFilterProxyModel::lessThan (left, right);

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
	}

}
}
}
