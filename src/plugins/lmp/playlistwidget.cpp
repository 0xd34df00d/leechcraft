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

#include "playlistwidget.h"
#include <QToolBar>
#include <QInputDialog>
#include <QFileDialog>
#include <QActionGroup>
#include <QToolButton>
#include <QMenu>
#include <QUndoStack>
#include <util/util.h>
#include "player.h"
#include "playlistdelegate.h"
#include "xmlsettingsmanager.h"
#include "core.h"
#include "playlistmanager.h"
#include "staticplaylistmanager.h"
#include "audiopropswidget.h"
#include "playlistundocommand.h"
#include "util.h"

namespace LeechCraft
{
namespace LMP
{
	PlaylistWidget::PlaylistWidget (QWidget *parent)
	: QWidget (parent)
	, PlaylistToolbar_ (new QToolBar ())
	, UndoStack_ (new QUndoStack (this))
	, Player_ (0)
	, ActionRemoveSelected_ (0)
	, ActionStopAfterSelected_ (0)
	, ActionShowTrackProps_ (0)
	, ActionShowAlbumArt_ (0)
	{
		Ui_.setupUi (this);

		Ui_.Playlist_->setItemDelegate (new PlaylistDelegate (Ui_.Playlist_, Ui_.Playlist_));
	}

	void PlaylistWidget::SetPlayer (Player *player)
	{
		Player_ = player;

		Ui_.Playlist_->setModel (Player_->GetPlaylistModel ());
		Ui_.Playlist_->expandAll ();

		connect (Ui_.Playlist_,
				SIGNAL (doubleClicked (QModelIndex)),
				Player_,
				SLOT (play (QModelIndex)));
		connect (Player_,
				SIGNAL (insertedAlbum (QModelIndex)),
				Ui_.Playlist_,
				SLOT (expand (QModelIndex)));

		Ui_.PlaylistLayout_->addWidget (PlaylistToolbar_);

		QAction *clearPlaylist = new QAction (tr ("Clear..."), this);
		clearPlaylist->setProperty ("ActionIcon", "edit-clear-list");
		connect (clearPlaylist,
				SIGNAL (triggered ()),
				Player_,
				SLOT (clear ()));
		PlaylistToolbar_->addAction (clearPlaylist);

		QAction *savePlaylist = new QAction (tr ("Save playlist..."), this);
		savePlaylist->setProperty ("ActionIcon", "document-save");
		connect (savePlaylist,
				SIGNAL (triggered ()),
				this,
				SLOT (handleSavePlaylist ()));
		PlaylistToolbar_->addAction (savePlaylist);

		QAction *loadFiles = new QAction (tr ("Load from disk..."), this);
		loadFiles->setProperty ("ActionIcon", "document-open");
		connect (loadFiles,
				SIGNAL (triggered ()),
				this,
				SLOT (loadFromDisk ()));
		PlaylistToolbar_->addAction (loadFiles);

		PlaylistToolbar_->addSeparator ();

		auto playButton = new QToolButton;
		playButton->setIcon (Core::Instance ().GetProxy ()->GetIcon ("view-media-playlist"));
		playButton->setPopupMode (QToolButton::InstantPopup);
		QMenu *playMode = new QMenu (tr ("Play mode"));
		playButton->setMenu (playMode);

		const std::vector<Player::PlayMode> modes = { Player::PlayMode::Sequential,
				Player::PlayMode::Shuffle, Player::PlayMode::RepeatTrack,
				Player::PlayMode::RepeatAlbum, Player::PlayMode::RepeatWhole };
		const std::vector<QString> names = { tr ("Sequential"),
				tr ("Shuffle"), tr ("Repeat track"),
				tr ("Repeat album"), tr ("Repeat whole") };
		PlayModesGroup_ = new QActionGroup (this);
		for (size_t i = 0; i < modes.size (); ++i)
		{
			QAction *action = new QAction (names [i], this);
			action->setProperty ("PlayMode", static_cast<int> (modes.at (i)));
			action->setCheckable (true);
			action->setChecked (!i);
			action->setActionGroup (PlayModesGroup_);
			playMode->addAction (action);

			connect (action,
					SIGNAL (triggered ()),
					this,
					SLOT (handleChangePlayMode ()));
		}
		connect (Player_,
				SIGNAL (playModeChanged (Player::PlayMode)),
				this,
				SLOT (handlePlayModeChanged (Player::PlayMode)));
		const int resumeMode = XmlSettingsManager::Instance ()
				.Property ("PlayMode", static_cast<int> (Player::PlayMode::Sequential)).toInt ();
		Player_->SetPlayMode (static_cast<Player::PlayMode> (resumeMode));

		PlaylistToolbar_->addWidget (playButton);

		PlaylistToolbar_->addAction (Util::CreateSeparator (this));
		auto undo = UndoStack_->createUndoAction (this);
		undo->setProperty ("ActionIcon", "edit-undo");
		undo->setShortcut (QKeySequence ("Ctrl+Z"));
		PlaylistToolbar_->addAction (undo);
		auto redo = UndoStack_->createRedoAction (this);
		redo->setProperty ("ActionIcon", "edit-redo");
		PlaylistToolbar_->addAction (redo);

		ActionRemoveSelected_ = new QAction (tr ("Delete from playlist"), Ui_.Playlist_);
		ActionRemoveSelected_->setProperty ("ActionIcon", "list-remove");
		ActionRemoveSelected_->setShortcut (Qt::Key_Delete);
		connect (ActionRemoveSelected_,
				SIGNAL (triggered ()),
				this,
				SLOT (removeSelectedSongs ()));

		ActionStopAfterSelected_ = new QAction (tr ("Stop after this track"), Ui_.Playlist_);
		ActionStopAfterSelected_->setProperty ("ActionIcon", "media-playback-stop");
		connect (ActionStopAfterSelected_,
				SIGNAL (triggered ()),
				this,
				SLOT (setStopAfterSelected ()));

		ActionShowTrackProps_ = new QAction (tr ("Show track properties"), Ui_.Playlist_);
		ActionShowTrackProps_->setProperty ("ActionIcon", "document-properties");
		connect (ActionShowTrackProps_,
				SIGNAL (triggered ()),
				this,
				SLOT (showTrackProps ()));

		ActionShowAlbumArt_ = new QAction (tr ("Show album art"), Ui_.Playlist_);
		ActionShowAlbumArt_->setProperty ("ActionIcon", "media-optical");
		connect (ActionShowAlbumArt_,
				SIGNAL (triggered ()),
				this,
				SLOT (showAlbumArt ()));
	}

	void PlaylistWidget::on_Playlist__customContextMenuRequested (const QPoint& pos)
	{
		const auto& idx = Ui_.Playlist_->indexAt (pos);
		if (!idx.isValid ())
			return;

		auto menu = new QMenu (Ui_.Playlist_);
		menu->addAction (ActionRemoveSelected_);
		if (idx.data (Player::Role::IsAlbum).toBool ())
			menu->addAction (ActionShowAlbumArt_);
		else
		{
			menu->addAction (ActionStopAfterSelected_);
			menu->addAction (ActionShowTrackProps_);
		}

		menu->setAttribute (Qt::WA_DeleteOnClose);

		menu->exec (Ui_.Playlist_->viewport ()->mapToGlobal (pos));
	}

	void PlaylistWidget::handleChangePlayMode ()
	{
		auto mode = sender ()->property ("PlayMode").toInt ();
		Player_->SetPlayMode (static_cast<Player::PlayMode> (mode));
		XmlSettingsManager::Instance ().setProperty ("PlayMode", mode);
	}

	void PlaylistWidget::handlePlayModeChanged (Player::PlayMode mode)
	{
		Q_FOREACH (QAction *action, PlayModesGroup_->actions ())
			if (action->property ("PlayMode").toInt () == static_cast<int> (mode))
			{
				action->setChecked (true);
				return;
			}
	}

	void PlaylistWidget::removeSelectedSongs ()
	{
		auto selModel = Ui_.Playlist_->selectionModel ();
		if (!selModel)
			return;

		auto indexes = selModel->selectedRows ();
		if (indexes.isEmpty ())
			indexes << Ui_.Playlist_->currentIndex ();
		indexes.removeAll (QModelIndex ());

		QList<Phonon::MediaSource> removedSources;
		const QString& title = indexes.size () == 1 ?
				tr ("Remove %1").arg (indexes.at (0).data ().toString ()) :
				tr ("Remove %n song(s)", 0, indexes.size ());

		Q_FOREACH (const auto& idx, indexes)
			removedSources << Player_->GetIndexSources (idx);

		auto cmd = new PlaylistUndoCommand (title, removedSources, Player_);
		UndoStack_->push (cmd);
	}

	void PlaylistWidget::setStopAfterSelected ()
	{
		auto index = Ui_.Playlist_->currentIndex ();
		if (!index.isValid ())
			return;

		Player_->SetStopAfter (index);
	}

	void PlaylistWidget::showTrackProps ()
	{
		const auto& index = Ui_.Playlist_->currentIndex ();
		const auto& info = index.data (Player::Role::Info).value<MediaInfo> ();
		if (info.LocalPath_.isEmpty ())
			return;

		AudioPropsWidget::MakeDialog ()->SetProps (info);
	}

	void PlaylistWidget::showAlbumArt ()
	{
		const auto& index = Ui_.Playlist_->currentIndex ();
		const auto& info = index.data (Player::Role::Info).value<MediaInfo> ();

		ShowAlbumArt (info.LocalPath_, QCursor::pos ());
	}

	void PlaylistWidget::handleSavePlaylist ()
	{
		const auto& name = QInputDialog::getText (this,
				tr ("Save playlist"),
				tr ("Enter name for the playlist:"));
		if (name.isEmpty ())
			return;

		auto mgr = Core::Instance ().GetPlaylistManager ()->GetStaticManager ();
		mgr->SaveCustomPlaylist (name, Player_->GetQueue ());
	}

	void PlaylistWidget::loadFromDisk ()
	{
		QStringList files = QFileDialog::getOpenFileNames (this,
				tr ("Load files"),
				QDir::homePath (),
				tr ("Music files (*.ogg *.flac *.mp3 *.wav);;All files (*.*)"));
		Player_->Enqueue (files);
	}
}
}
