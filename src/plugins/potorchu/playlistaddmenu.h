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

#ifndef PLAYLISTADDMENU_H
#define PLAYLISTADDMENU_H

#include <QMenu>
#include <QFileInfoList>
#include "playlistview.h"

namespace LeechCraft
{
	namespace Potorchu
	{
		class PlayListAddMenu : public QMenu
		{
			Q_OBJECT
			PlayListView *PlayListView_;
		public:
			PlayListAddMenu (PlayListView *playListView, QWidget* parent);
		private slots:
			void handleAddUrl ();
			void handleAddFolder ();
			void handleAddFiles ();
		private:
			QFileInfoList StoragedFiles (const QString& path);
		};
	}
}
#endif // PLAYLISTADDMENU_H
