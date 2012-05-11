/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
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

#pragma once

#include <QSystemTrayIcon>
#include "mediainfo.h"

namespace LeechCraft
{
namespace LMP
{
	class LMPSystemTrayIcon : public QSystemTrayIcon
	{
		Q_OBJECT

		MediaInfo CurrentSong_;
	public:
		LMPSystemTrayIcon (const QIcon& icon, QObject *parent = 0);
	protected:
		bool event (QEvent *event);

	public slots:
		void handleSongChanged (const MediaInfo& song);

	signals:
		void changedVolume (qreal delta);
	};
}
}
