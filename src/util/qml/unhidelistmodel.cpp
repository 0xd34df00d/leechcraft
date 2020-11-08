/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "unhidelistmodel.h"

namespace LC::Util
{
	UnhideListModel::UnhideListModel (QObject *parent)
	: RoleNamesMixin<QStandardItemModel> (parent)
	{
		setRoleNames ({
				{ Roles::ItemClass, QByteArrayLiteral ("itemClass") },
				{ Roles::ItemName, QByteArrayLiteral ("itemName") },
				{ Roles::ItemDescription, QByteArrayLiteral ("itemDescr") },
				{ Roles::ItemIcon, QByteArrayLiteral ("itemIcon") },
			});
	}
}
