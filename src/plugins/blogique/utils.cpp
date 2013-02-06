/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "utils.h"

namespace LeechCraft
{
namespace Blogique
{
namespace Utils
{
	QList<QStandardItem*> CreateEntriesViewRow (const Entry& entry)
	{
		QStandardItem *dateItem = new QStandardItem (entry.Date_.date ()
		.toString (Qt::SystemLocaleShortDate) +
		" " +
		entry.Date_.time ().toString ("hh:mm"));
		dateItem->setData (entry.EntryId_, Utils::EntryIdRole::DBIdRole);
		dateItem->setEditable (false);
		dateItem->setData (entry.Subject_, Qt::ToolTipRole);
		QStandardItem *itemSubj = new QStandardItem (entry.Subject_);
		itemSubj->setEditable (false);
		itemSubj->setData (entry.Subject_, Qt::ToolTipRole);

		return { dateItem, itemSubj };
	}
}
}
}
