/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011  Minh Ngo
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

#include "nowplayingdelegate.h"
#include <QPainter>
#include <QDebug>
#include "playlistmodel.h"

namespace LeechCraft
{
namespace Laure
{
	NowPlayingDelegate::NowPlayingDelegate (QObject *parent)
	: QItemDelegate (parent)
	{
		
	}
	
	void NowPlayingDelegate::paint (QPainter *painter, const QStyleOptionViewItem& option,
				const QModelIndex& index) const
	{
		const bool played = index.data (PlayListModel::IsPlayingRole).toBool ();
		if (played)
			painter->fillRect (option.rect, Qt::gray);

		QItemDelegate::paint (painter, option, index);
	}
}
}