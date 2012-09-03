/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
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
#include "ui_playlistwidget.h"
#include "player.h"

class QToolBar;
class QActionGroup;
class QUndoStack;
class QSortFilterProxyModel;

namespace LeechCraft
{
namespace LMP
{
	class Player;

	class PlaylistWidget : public QWidget
	{
		Q_OBJECT

		Ui::PlaylistWidget Ui_;
		QToolBar *PlaylistToolbar_;
		QActionGroup *PlayModesGroup_;
		QSortFilterProxyModel *PlaylistFilter_;

		QUndoStack *UndoStack_;

		Player *Player_;

		QAction *ActionRemoveSelected_;
		QAction *ActionStopAfterSelected_;
		QAction *ActionShowTrackProps_;
		QAction *ActionShowAlbumArt_;
		QAction *ActionMoveUp_;
		QAction *ActionMoveDown_;
	public:
		PlaylistWidget (QWidget* = 0);

		void SetPlayer (Player*);
	private:
		void InitToolbarActions ();
		void SetPlayModeButton ();
		void SetSortOrderButton ();
		void InitViewActions ();
		void SelectSources (const QList<Phonon::MediaSource>&);
	private slots:
		void on_Playlist__customContextMenuRequested (const QPoint&);
		void handleChangePlayMode ();
		void handlePlayModeChanged (Player::PlayMode);

		void play (const QModelIndex&);
		void expand (const QModelIndex&);

		void handleBufferStatus (int);

		void handleStdSort ();

		void removeSelectedSongs ();
		void setStopAfterSelected ();
		void showTrackProps ();

		void showAlbumArt ();

		void handleMoveUp ();
		void handleMoveDown ();

		void handleSavePlaylist ();
		void loadFromDisk ();
		void addURL ();

		void updateStatsLabel ();
	};
}
}
