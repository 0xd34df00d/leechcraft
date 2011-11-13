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

#ifndef PLUGINS_LAURE_PLAYLISTWIDGET_H
#define PLUGINS_LAURE_PLAYLISTWIDGET_H
#include <QWidget>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/structures.h>
#include "playlistview.h"

class QDockWidget;
class QGridLayout;

namespace LeechCraft
{
namespace Laure
{
	class PlayListView;
	class PlayListModel;
	
	/** @brief Provides a playlist widget with control tool buttons.
	 * 
	 * @author Minh Ngo <nlminhtl@gmail.com>
	 */
	class PlayListWidget : public QWidget
	{
		Q_OBJECT
		
		QGridLayout *GridLayout_;
		PlayListModel *PlayListModel_;
		PlayListView *PlayListView_;
		QToolBar *ActionBar_;
	public:
		/** @brief Constructs a new PlayListWidget class
		 * with the given parent.
		 */
		PlayListWidget (QWidget* = 0);
	public slots:
		/** @brief This slot's called when the media file has added to
		 * VLCWrapper.
		 * 
		 * @param[in] meta Media meta info
		 * @param[in] fileName Media file location
		 * 
		 * @sa VLCWrapper
		 */
		void handleItemAdded (const MediaMeta& meta, const QString& fileName);
		
		/** @brief This slot's called when the media item has played
		 * in VLCWrapper.
		 * 
		 * @param[in] row Item index
		 * 
		 * @sa VLCWrapper
		 */
		void handleItemPlayed (int row);
	private slots:
		void handleExportPlayList ();
	signals:
		/** @brief This signal notifies that the media file needs to be
		 * added to VLCWrapper.
		 * 
		 * @param[out] location Media file location
		 * 
		 * @sa VLCWrapper
		 */
		void itemAddedRequest (const QString& location);
		
		/** @brief This signal's emmited when the item index's removed.
		 * 
		 * @param[out] index Item index
		 */
		void itemRemoved (int);
		
		/** @brief This signal notifies that the given item needs to be played.
		 * 
		 * @param[out] index The index of the item to play.
		 */
		void playItem (int);
		
		/** @brief This signal is emitted by plugin to notify the Core and
		 * other plugins about an entity.
		 *
		 * In this case, the plugin doesn't care what would happen next to
		 * the entity after the announcement and whether it would be catched
		 * by any other plugin at all. This is the opposite to the semantics
		 * of delegateEntity().
		 *
		 * This signal is typically emitted, for example, when a plugin has
		 * just finished downloading something and wants to notify other
		 * plugins about newly created files.
		 *
		 * This signal is asynchronous: the handling happends after the
		 * control gets back to the event loop.
		 *
		 * @note This function is expected to be a signal in subclasses.
		 *
		 * @param[out] entity The entity.
		 */
		void gotEntity (const Entity&);
		
		/** @brief This signal is emitted by plugin to delegate the entity
		 * to an another plugin.
		 *
		 * In this case, the plugin actually cares whether the entity would
		 * be handled. This signal is typically used, for example, to
		 * delegate a download request.
		 *
		 * id and provider are used in download delegation requests. If
		 * these parameters are not NULL and the entity is handled, they are
		 * set to the task ID returned by the corresponding IDownload
		 * instance and the main plugin instance of the handling plugin,
		 * respectively. Thus, setting the id to a non-NULL value means that
		 * only downloading may occur as result but no handling.
		 *
		 * Nevertheless, if you need to enable entity handlers to handle
		 * your request as well, you may leave the id parameter as NULL and
		 * just set the provider to a non-NULL value.
		 *
		 * @note This function is expected to be a signal in subclasses.
		 *
		 * @param[out] entity The entity to delegate.
		 * @param[in] id The pointer to the variable that would contain the
		 * task ID of this delegate request, or NULL.
		 * @param[in] provider The pointer to the main plugin instance of
		 * the plugin that handles this delegate request, or NULL.
		 */
		void delegateEntity (const Entity&, int*, QObject**);
		
		/** @brief This signal's emmited when the playback mode is changed.
		 * 
		 * @param[out] mode New playback mode
		 * 
		 * @sa PlaybackMode
		 */
		void playbackModeChanged (PlaybackMode mode);
	};
}
}

#endif // PLUGINS_LAURE_PLAYLISTWIDGET_H
