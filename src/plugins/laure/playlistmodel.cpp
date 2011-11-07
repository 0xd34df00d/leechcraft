/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011 Minh Ngo
 * Copyright (C) 2006-2011  Georg Rudoy
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

#include "playlistmodel.h"

namespace LeechCraft
{
namespace Laure
{
	const int PlayListColumnCount = 6;
	
	PlayListModel::PlayListModel (QObject* parent)
	: QStandardItemModel (parent)
	{
		setColumnCount (PlayListColumnCount);
		HeaderNames_ << tr ("Artist")
				<< tr ("Title")
				<< tr ("Album")
				<< tr ("Genre")
				<< tr ("Date");
		for (int i = 1; i < PlayListColumnCount; ++i)
			setHeaderData (i, Qt::Horizontal, HeaderNames_ [i - 1]);
	}
	
	Qt::ItemFlags PlayListModel::flags (const QModelIndex& index) const
	{
		return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDropEnabled;
	}
}
}

