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

#ifndef PLUGINS_LAURE_LAUREWIDGET_H
#define PLUGINS_LAURE_LAUREWIDGET_H
#include <QWidget>
#include <interfaces/ihavetabs.h>
#include <interfaces/iinfo.h>
#include "ui_laurewidget.h"

class QToolBar;
class QUrl;

namespace LeechCraft
{
namespace Laure
{
	class Player;
	class PlayListWidget;
	class PlayPauseAction;
	
	/** @brief Represents a tab in LeechCraft tabs system.
	 * 
	 * @author Minh Ngo <nlminhtl@gmail.com>
	 */
	class LaureWidget : public QWidget
				, public ITabWidget
	{
		Q_OBJECT
		Q_INTERFACES (ITabWidget)
		
		static QObject *S_ParentMultiTabs_;
		
		QToolBar *ToolBar_;
		Ui::LaureWidget Ui_;
		VLCWrapper *VLCWrapper_;
	public:
		/** @brief Constructs a new LaureWidget tab
		 * with the given parent and flags.
		 */
		LaureWidget (QWidget *parent = 0, Qt::WindowFlags f = 0);
		
		/** @brief This method sets the pointer to the Laure plugin instance object
		 * that is later returned by the ITabWidget::GetParentPlugin().
		 * 
		 * @sa ITabWidget::GetParentPlugin()
		 */
		static void SetParentMultiTabs (QObject*);
		
		/** @brief Returns the description of the tab class of this tab.
		 * 
		 * The entry must be the same as the one with the same TabClass_
		 * returned from the IHaveTabs::GetTabClasses().
		 * 
		 * @return The tab class description.
		 * 
		 * @sa IHavetabs::GetTabClasses()
		 */
		TabClassInfo GetTabClassInfo () const;
		
		/** @brief Returns the pointer to the plugin this tab belongs to.
		 * 
		 * The returned object must implement IHaveTabs and must be the one
		 * that called IHaveTabs::addNewTab() with this tab as the
		 * parameter.
		 * 
		 * @return The pointer to the plugin that this tab belongs to.
		 */
		QObject* ParentMultiTabs ();
		
		/** @brief Requests to remove the tab.
		 * 
		 * This method is called by LeechCraft Core (or other plugins) when
		 * this tab should be closed, for example, when user clicked on the
		 * 'x' in the tab bar. The tab may ask the user if he really wants
		 * to close the tab, for example. The tab is free to ignore the
		 * close request (in this case nothing should be done at all) or
		 * accept it by emitting IHavetabs::removeTab() signal, passing this
		 * tab widget as its parameter.
		 */
		void Remove ();
		
		/** @brief Requests tab's toolbar.
		 * 
		 * This method is called to obtain the tab toolbar. In current
		 * implementation, it would be shown on top of the LeechCraft's main
		 * window.
		 * 
		 * If the tab has no toolbar, 0 should be returned.
		 * 
		 * @return The tab's toolbar, or 0 if there is no toolbar.
		 */
		QToolBar* GetToolBar () const;
	
	protected:
		void keyPressEvent (QKeyEvent *);
	private:
		void InitCommandFrame ();
		void InitToolBar ();
	signals:
		/** @brief This signal's emitted to notify the Core
		 * that this tab needs to be closed.
		 */
		void needToClose ();
		
		/** @brief This signal's emmited when the PlayPauseAction
		 * is clicked.
		 */
		void playPause ();
		
		/** @brief This signal's emmited for sending media meta info
		 * to the desired destination.
		 * 
		 * @param[out] mediameta media meta info
		 */
		void currentTrackMeta (const MediaMeta&);
		
		/** @brief This signal's emmited when the current track's finished.
		 */
		void trackFinished ();
		
		/** @brief This signal's emmited when the media item's added to
		 * the playlist.
		 *
		 * @param[out] location media file location
		 */
		void addItem (const QString&);
		
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
	public slots:
		/** @brief This handle's called for adding media files to the
		 * playlist.
		 * 
		 * @param[in] location media file location.
		 */
		void handleOpenMediaContent (const QString&);
	private slots:
		void handleOpenFile ();
		void handleOpenURL ();
		void updateInterface ();
		void handleVideoMode (bool);
	};
}
}

#endif // PLUGINS_LAURE_LAUREWIDGET_H
