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

#ifndef PLUGINS_LAURE_PLAYLISTVIEW_H
#define PLUGINS_LAURE_PLAYLISTVIEW_H

#include <QTreeView>
#include "vlcwrapper.h"

class QKeyEvent;

namespace LeechCraft
{
namespace Laure
{
	class PlayListModel;
	
	/** @brief Provides a model/view implementation of a playlist view.
	 * 
	 * @author Minh Ngo <nlminhtl@gmail.com>
	 */
	class PlayListView : public QTreeView
	{
		Q_OBJECT
		
		PlayListModel *PlayListModel_;
		int CurrentItem_;
	public:
		/** @brief Constructs a new PlayListView class
		 * with the given model and parent.
		 * 
		 * @param[in] model Playlist model
		 * 
		 * @sa PlayListModel
		 */
		PlayListView (PlayListModel *model, QWidget* = 0);
		
		/** @brief This method adds the item into the playlist.
		 * 
		 * @param[in] item Media meta info
		 * @param[in] fileName Media file location
		 * 
		 * @sa MediaMeta
		 */
		void AddItem (const MediaMeta& item, const QString& fileName);
		
		/** @brief This method sets the playing item.
		 * 
		 * @param[in] row  Item index
		 */
		void Play (int row);
	protected:
		void keyPressEvent (QKeyEvent*);
	public slots:
		/** @brief This slot's called to select the item row.
		 * 
		 * @param[in] row Item index
		 */
		void selectRow (int row);
		
		/** @brief This slot's called to remove selected rows.
		 */
		void removeSelectedRows ();
	private slots:
		void handleDoubleClicked (const QModelIndex&);
	signals:
		/** @brief This signal's emmited when the item index's removed.
		 * 
		 * @param[out] index Item index
		 */
		void itemRemoved (int index);
		
		/** @brief This signal notifies that the given item needs to be played.
		 * 
		 * @param[out] index The index of the item to play.
		 */
		void playItem (int index);
	};
}
}
#endif // PLUGINS_LAURE_PLAYLISTVIEW_H
