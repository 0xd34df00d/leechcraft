/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "contactslistview.h"
#include "core.h"

namespace LC::Azoth
{
	QRect ContactsListView::visualRect (const QModelIndex& index) const
	{
		auto rect = QTreeView::visualRect (index);
		if (index.data (Core::CLREntryType).value<Core::CLEntryType> () == Core::CLETContact)
			rect.setLeft (0);
		return rect;
	}
}
