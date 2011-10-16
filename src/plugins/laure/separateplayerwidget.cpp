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

#include "separateplayerwidget.h"

namespace LeechCraft
{
namespace Laure
{
	SeparatePlayerWidget::SeparatePlayerWidget (libvlc_media_player_t *MP,
			QWidget *playerWidget, QWidget *parent)
	: QWidget (parent)
	, PlayerWidget_ (playerWidget)
	, MP_ (MP)
	{
		setPalette (QPalette (Qt::black));
		changeWidget (this);
	}

	void SeparatePlayerWidget::closeEvent (QCloseEvent *event)
	{
		changeWidget (PlayerWidget_);
		setParent (PlayerWidget_);
	}
	
	void SeparatePlayerWidget::changeWidget (QWidget *w)
	{
		int time = libvlc_media_player_get_time (MP_);
		libvlc_media_player_stop (MP_);
		libvlc_media_player_set_xwindow (MP_, w->winId ());
		libvlc_media_player_play (MP_);
		libvlc_media_player_set_time (MP_, time);
	}
}
}