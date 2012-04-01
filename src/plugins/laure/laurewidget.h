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
#include <memory>
#include <QWidget>
#include <interfaces/ihavetabs.h>
#include <interfaces/iinfo.h>
#include "ui_laurewidget.h"

class QToolBar;
class QUrl;
class QMenu;

namespace LeechCraft
{
namespace Laure
{
	class Player;
	class PlayListWidget;
	class PlayPauseAction;
	class SeparatePlayer;

	/** @brief Represents a tab in LeechCraft tabs system.
	 *
	 * @author Minh Ngo <nlminhtl@gmail.com>
	 *
	 * @sa ITabWidget
	 */
	class LaureWidget : public QWidget
				, public ITabWidget
	{
		Q_OBJECT
		Q_INTERFACES (ITabWidget)

		static QObject *S_ParentMultiTabs_;

		QToolBar *ToolBar_;
		Ui::LaureWidget Ui_;
		std::shared_ptr<VLCWrapper> VLCWrapper_;
		std::shared_ptr<SeparatePlayer> SeparatePlayer_;
		QAction *DetachedVideo_;
		QAction *PlayListAction_;
		QAction *SubtitleAction_;
		QMenu *SubtitleMenu_;
		PlayPauseAction *ActionPlay_;
	public:
		/** @brief Constructs a new LaureWidget tab
		 * with the given parent and flags.
		 */
		LaureWidget (QWidget *parent = 0, Qt::WindowFlags f = 0);
		~LaureWidget ();

		static void SetParentMultiTabs (QObject*);
		TabClassInfo GetTabClassInfo () const;
		QObject* ParentMultiTabs ();
		void Remove ();
		QToolBar* GetToolBar () const;

	protected:
		void keyPressEvent (QKeyEvent*);
	private:
		void InitCommandFrame ();
		void InitToolBar ();
	signals:
		/** @brief Is emitted to notify the Core that this tab needs to
		 * be closed.
		 */
		void needToClose ();

		/** @brief Is emitted when the PlayPauseAction is clicked.
		 */
		void playPause ();

		/** @brief Is emitted when the media item needs to be added to
		 * the playlist.
		 *
		 * @param[out] location Media file location.
		 */
		void addItem (const QString& location);

		void gotEntity (const Entity&);
		void delegateEntity (const Entity&, int*, QObject**);
	public slots:
		/** @brief Is called for adding media files to the playlist.
		 *
		 * @param[in] location Media file location.
		 */
		void handleOpenMediaContent (const QString& location);
		
		void updateSubtitleMenu (const MediaMeta& meta);
	private slots:
		void handleOpenFile ();
		void handleOpenURL ();
		void updateInterface ();
		void handleVideoMode (bool);
		void handleDetachPlayer (bool);
		void handleSeparatePlayerClosed ();
		void subtitleDialog ();
		void showSubtitleMenu ();
		void handlePlayListWidgetDoubleClicked ();
	};
}
}