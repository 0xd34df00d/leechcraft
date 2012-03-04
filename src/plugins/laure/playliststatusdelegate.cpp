/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011-2012 Minh Ngo
 * Copyright (C) 2006-2012  Georg Rudoy
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

#include "playliststatusdelegate.h"
#include <QPainter>
#include <interfaces/core/icoreproxy.h>
#include "playlistview.h"
#include "core.h"

namespace LeechCraft
{
namespace Laure
{
	PlayListStatusDelegate::PlayListStatusDelegate (QObject *parent)
	: QStyledItemDelegate (parent)
	, PlayPixmap_ (Core::Instance ().GetProxy ()->GetIcon ("media-playback-start").pixmap (16, 16))
	{
	}
	
	void PlayListStatusDelegate::paint (QPainter *painter,
			const QStyleOptionViewItem& option, const QModelIndex& id) const
	{
		QStyledItemDelegate::paint (painter, option, id);
		const bool played = id.sibling (id.row (), PlayListColumns::StatusColumn)
				.data (IsPlayingRole).toBool ();
				
		if (played)
			painter->drawPixmap (option.rect, PlayPixmap_);
	}
}
}