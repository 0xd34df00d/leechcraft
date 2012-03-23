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
#include <QWidget>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/structures.h>
#include "playlistview.h"

class QDockWidget;
class QGridLayout;
class QStandardItemModel;
class QStandardItem;

namespace LeechCraft
{
namespace Laure
{
	class PlayListView;
	
	/** @brief Provides a playlist widget with control tool buttons.
	 * 
	 * @author Minh Ngo <nlminhtl@gmail.com>
	 */
	class PlayListWidget : public QWidget
	{
		Q_OBJECT
		
		QGridLayout *GridLayout_;
		QStandardItemModel *PlayListModel_;
		PlayListView *PlayListView_;
		QToolBar *ActionBar_;
		std::shared_ptr<VLCWrapper> VLCWrapper_;
	public:
		/** @brief Constructs a new PlayListWidget class
		 * with the given parent.
		 */
		PlayListWidget (QWidget* = 0);

		void Init (std::shared_ptr<VLCWrapper> wrapper);
	public slots:
		/** @brief Is called when the media file has added to
		 * VLCWrapper.
		 * 
		 * @param[in] meta Media meta info.
		 * @param[in] fileName Media file location.
		 * 
		 * @sa VLCWrapper
		 * @sa MediaMeta
		 */
		void handleItemAdded (const MediaMeta& meta, const QString& fileName);
		
		/** @brief Is called when the media item has played
		 * in VLCWrapper.
		 * 
		 * @param[in] row Item index.
		 * 
		 * @sa VLCWrapper
		 */
		void handleItemPlayed (int row);
	private slots:
		void handleExportPlayList ();
		void handleItemChanged (QStandardItem*);
	signals:
		/** @brief Notifies that the media file needs to be
		 * added to VLCWrapper.
		 * 
		 * @param[out] location Media file location.
		 * 
		 * @sa VLCWrapper
		 */
		void itemAddedRequest (const QString& location);
		
		/** @brief Notifies that the media meta needs to be changed.
		 * 
		 * @param[in] type Media type.
		 * @param[in] value New media meta info.
		 * @param[in] index Playlist item index.
		 */
		void metaChangedRequest (libvlc_meta_t type, const QString& value,
				int index);

		void gotEntity (const Entity&);
		void delegateEntity (const Entity&, int*, QObject**);
		
		/** @brief Is emitted when the playback mode is changed.
		 * 
		 * @param[out] mode New playback mode.
		 * 
		 * @sa PlaybackMode
		 */
		void playbackModeChanged (PlaybackMode mode);
		void doubleClicked ();
	};
}
}
