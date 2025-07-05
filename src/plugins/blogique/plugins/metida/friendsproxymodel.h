/**********************************************************************
 *  LeechCraft - modular cross-platform feature rich internet client.
 *  Copyright (C) 2010-2013  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <util/models/fixedstringfilterproxymodel.h>

namespace LC
{
namespace Blogique
{
namespace Metida
{
	class FriendsProxyModel : public Util::FixedStringFilterProxyModel
	{
	public:
		enum FriendsRoles
		{
			FRFriendStatus = Qt::UserRole + 1
		};

		enum Columns
		{
			CNickname,
			CFriendStatus,
			CUsername,
			CBirthday
		};

		enum FriendStatus
		{
			FSFriendOf,
			FSMyFriend,
			FSBothFriends
		};

		explicit FriendsProxyModel (QObject *parent = nullptr);
	protected:
		bool lessThan (const QModelIndex& left, const QModelIndex& right) const override;
	};
}
}
}
