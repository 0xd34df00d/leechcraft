/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include <interfaces/core/ihookproxy.h>
#include <interfaces/core/icoreproxyfwd.h>
#include "ui_playlistwidget.h"
#include "player.h"

class QToolBar;
class QActionGroup;
class QUndoStack;
class QSortFilterProxyModel;

namespace LC
{
namespace LMP
{
	class Player;

	class PlaylistWidget : public QWidget
	{
		Q_OBJECT

		Ui::PlaylistWidget Ui_;
		QToolBar * const PlaylistToolbar_;
		QActionGroup *PlayModesGroup_ = nullptr;
		QSortFilterProxyModel * const PlaylistFilter_;

		QUndoStack * const UndoStack_;

		ICoreProxy_ptr Proxy_;
		Player *Player_ = nullptr;

		QAction *ActionDownloadTrack_ = nullptr;

		QAction *ActionRemoveSelected_ = nullptr;

		QAction *ActionStopAfterSelected_ = nullptr;
		QAction *ActionAddToOneShot_ = nullptr;
		QAction *ActionRemoveFromOneShot_ = nullptr;
		QAction *ActionMoveOneShotUp_ = nullptr;
		QAction *ActionMoveOneShotDown_ = nullptr;

		QAction *ActionShowTrackProps_ = nullptr;
		QAction *ActionShowAlbumArt_ = nullptr;
		QAction *ActionMoveTop_ = nullptr;
		QAction *ActionMoveUp_ = nullptr;
		QAction *ActionMoveDown_ = nullptr;
		QAction *ActionMoveBottom_ = nullptr;

		QMenu *TrackActions_ = nullptr;
		QMenu *ExistingTrackActions_ = nullptr;

		QAction *MoveUpButtonAction_ = nullptr;
		QAction *MoveDownButtonAction_ = nullptr;

		QAction *ActionToggleSearch_ = nullptr;

		QList<AudioSource> NextResetSelect_;
	public:
		PlaylistWidget (QWidget* = nullptr);

		void SetPlayer (Player*, const ICoreProxy_ptr&);
	private:
		void InitCommonActions ();
		void InitToolbarActions ();
		void SetPlayModeButton ();
		void SetSortOrderButton ();
		void InitViewActions ();

		void EnableMoveButtons (bool);

		QList<AudioSource> GetSelected () const;
		void SelectSources (const QList<AudioSource>&);
	public slots:
		void focusIndex (const QModelIndex&);
	private slots:
		void on_Playlist__customContextMenuRequested (const QPoint&);
		void handleChangePlayMode ();
		void handlePlayModeChanged (Player::PlayMode);

		void play (const QModelIndex&);
		void expand (const QModelIndex&);
		void expandAll ();
		void checkSelections ();

		void handleBufferStatus (int);
		void handleSongChanged (const MediaInfo&);

		void handleStdSort ();
		void handleCustomSort ();

		void savePlayScrollPosition ();

		void removeSelectedSongs ();

		void setStopAfterSelected ();
		void addToOneShot ();
		void removeFromOneShot ();
		void moveOneShotUp ();
		void moveOneShotDown ();

		void showTrackProps ();

		void showAlbumArt ();

		void initPerformAfterTrackStart ();
		void initPerformAfterTrackStop ();
		void handleExistingTrackAction (QAction*);

		void handleMoveUp ();
		void handleMoveTop ();
		void handleMoveDown ();
		void handleMoveBottom();

		void handleSavePlaylist ();
		void loadFromDisk ();
		void addURL ();

		bool updateDownloadAction ();
		void handleDownload ();

		void updateStatsLabel ();
	signals:
		void hookPlaylistContextMenuRequested (LC::IHookProxy_ptr,
				QMenu*,
				LC::LMP::MediaInfo);
	};
}
}
