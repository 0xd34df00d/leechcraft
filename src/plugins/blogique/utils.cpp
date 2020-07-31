/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "utils.h"
#include <QLocale>

namespace LC
{
namespace Blogique
{
namespace Utils
{
	QList<QStandardItem*> CreateEntriesViewRow (const Entry& entry)
	{
		auto dateItem = new QStandardItem (QLocale {}.toString (entry.Date_.date (), QLocale::ShortFormat) +
				" " +
				entry.Date_.time ().toString ("hh:mm"));
		dateItem->setData (entry.EntryId_, Utils::EntryIdRole::DBIdRole);
		dateItem->setEditable (false);
		dateItem->setData (entry.Subject_, Qt::ToolTipRole);
		auto itemSubj = new QStandardItem (entry.Subject_);
		itemSubj->setEditable (false);
		itemSubj->setData (entry.Subject_, Qt::ToolTipRole);

		return { dateItem, itemSubj };
	}
}
}
}
