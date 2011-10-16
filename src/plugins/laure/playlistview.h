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

#ifndef PLAYLISTVIEW_H
#define PLAYLISTVIEW_H

#include <QListView>
#include "playlistmodel.h"

class QKeyEvent;

namespace LeechCraft
{
namespace Laure
{
	class PlayListModel;
	class PlayListView : public QListView
	{
		Q_OBJECT
		PlayListModel *PlayListModel_;
	public:
		PlayListView (QWidget *parent = 0);
		bool SetPlayList (libvlc_media_list_t *ML);
		bool SetInstance (libvlc_instance_t *VLCInstance);
		void addItem (const QString& item);
		
		void keyPressEvent (QKeyEvent *event);
		int CurrentIndex () const;
		int RowCount () const;
		
		libvlc_media_t *CurrentMedia ();
	private:
		void moveSelect (int x, int y);
	public slots:
		void SetCurrentIndex (int val);
		void removeSelectedRows ();
	private slots:
		void handleDoubleClicked (const QModelIndex& index);
	signals:
		void playItem (int item);
	};
}
}
#endif // PLAYLISTVIEW_H
