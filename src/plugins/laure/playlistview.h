/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011-2012  Minh Ngo
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

#pragma once
#include <QTreeView>
#include "vlcwrapper.h"

class QKeyEvent;
class QStandardItemModel;
class QMouseEvent;

namespace LeechCraft
{
namespace Laure
{
	enum Roles
	{
		IsPlayingRole = Qt::UserRole + 1
	};
	
	enum PlayListColumns
	{
		StatusColumn,
		URLColumn,
		ArtistColumn,
		TitleColumn,
		AlbumColumn,
		GenreColumn,
		DateColumn,
		QueueColumn,
		LengthColumn,
		MAX
	};
	
	/** @brief Provides a model/view implementation of a playlist view.
	 * 
	 * @author Minh Ngo <nlminhtl@gmail.com>
	 */
	class PlayListView : public QTreeView
	{
		Q_OBJECT
		
		QStandardItemModel *PlayListModel_;
		int CurrentItem_;
		int NotHiddenColumnCount_;
		std::shared_ptr<VLCWrapper> VLCWrapper_;
		QMap<PlayListColumns, QByteArray> HeaderProperties_;
	public:
		/** @brief Constructs a new PlayListView class
		 * with the given model and parent.
		 * 
		 * @param[in] model Playlist model.
		 */
		PlayListView (QStandardItemModel *model, QWidget* = 0);
		
		void Init (std::shared_ptr<VLCWrapper> wrapper);
		
		/** @brief Adds the item into the playlist.
		 * 
		 * @param[in] item Media meta info.
		 * @param[in] fileName Media file location.
		 * 
		 * @sa MediaMeta
		 */
		void AddItem (const MediaMeta& item, const QString& fileName);
		
		/** @brief Sets the playing item.
		 * 
		 * @param[in] row  Item index.
		 */
		void MarkPlayingItem (int row);
	protected:
		void keyPressEvent (QKeyEvent*);
	public slots:
		/** @brief Is called to select the item row.
		 * 
		 * @param[in] row Item index.
		 */
		void selectRow (int row);
		
		/** @brief Is called to remove selected rows.
		 */
		void removeSelectedRows ();
	private slots:
		void handleDoubleClicked (const QModelIndex&);
		void handleHideHeaders ();
		void handleHeaderMenu (const QPoint& point);
		void handleMenu (const QPoint& point);
		void handleSectionResized (int logicalIndex, int oldSize, int newSize);
	private:
		void UpdateQueueIndexes ();
		
	signals:
		/** @brief Is emitted when the item index is removed.
		 * 
		 * @param[out] index Item index.
		 */
		void itemRemoved (int index);
		
		/** @brief Notifies that the given item needs to be played.
		 * 
		 * @param[out] index The index of the item to play.
		 */
		void playItem (int index);
	};
}
}