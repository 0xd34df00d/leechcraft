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

#include "playertab.h"
#include <algorithm>
#include <QToolBar>
#include <QFileDialog>
#include <QFileSystemModel>
#include <QMenu>
#include <QToolButton>
#include <phonon/seekslider.h>
#include "player.h"
#include "playlistdelegate.h"
#include "util.h"
#include "core.h"
#include "localcollection.h"
#include "collectiondelegate.h"
#include "xmlsettingsmanager.h"
#include <util/util.h>

namespace LeechCraft
{
namespace LMP
{
	PlayerTab::PlayerTab (const TabClassInfo& info, QObject *plugin, QWidget *parent)
	: QWidget (parent)
	, Plugin_ (plugin)
	, TC_ (info)
	, FSModel_ (new QFileSystemModel (this))
	, Player_ (new Player (this))
	, PlaylistToolbar_ (new QToolBar ())
	, TabToolbar_ (new QToolBar ())
	{
		Ui_.setupUi (this);
		Ui_.MainSplitter_->setStretchFactor (0, 2);
		Ui_.MainSplitter_->setStretchFactor (1, 1);

		connect (Player_,
				SIGNAL (songChanged (MediaInfo)),
				this,
				SLOT (handleSongChanged (MediaInfo)));
		connect (Core::Instance ().GetLocalCollection (),
				SIGNAL (scanStarted (int)),
				Ui_.ScanProgress_,
				SLOT (setMaximum (int)));
		connect (Core::Instance ().GetLocalCollection (),
				SIGNAL (scanProgressChanged (int)),
				this,
				SLOT (handleScanProgress (int)));
		Ui_.ScanProgress_->hide ();
		handleSongChanged (MediaInfo ());

		SetupToolbar ();
		SetupCollection ();
		SetupFSBrowser ();
		SetupPlaylist ();
	}

	TabClassInfo PlayerTab::GetTabClassInfo () const
	{
		return TC_;
	}

	QObject* PlayerTab::ParentMultiTabs ()
	{
		return Plugin_;
	}

	void PlayerTab::Remove ()
	{
		emit removeTab (this);
	}

	QToolBar* PlayerTab::GetToolBar () const
	{
		return TabToolbar_;
	}

	void PlayerTab::SetupToolbar ()
	{
		QAction *previous = new QAction (tr ("Previous track"), this);
		previous->setProperty ("ActionIcon", "media-skip-backward");
		connect (previous,
				SIGNAL (triggered ()),
				Player_,
				SLOT (previousTrack ()));
		TabToolbar_->addAction (previous);

		QAction *pause= new QAction (tr ("Play/pause"), this);
		pause->setProperty ("ActionIcon", "media-playback-pause");
		connect (pause,
				SIGNAL (triggered ()),
				Player_,
				SLOT (togglePause ()));
		TabToolbar_->addAction (pause);

		QAction *stop = new QAction (tr ("Stop"), this);
		stop->setProperty ("ActionIcon", "media-playback-stop");
		connect (stop,
				SIGNAL (triggered ()),
				Player_,
				SLOT (stop ()));
		TabToolbar_->addAction (stop);

		QAction *next = new QAction (tr ("Next track"), this);
		next->setProperty ("ActionIcon", "media-skip-forward");
		connect (next,
				SIGNAL (triggered ()),
				Player_,
				SLOT (nextTrack ()));
		TabToolbar_->addAction (next);

		TabToolbar_->addSeparator ();

		auto seekSlider = new Phonon::SeekSlider (Player_->GetSourceObject ());
		seekSlider->setTracking (false);
		TabToolbar_->addWidget (seekSlider);
	}

	void PlayerTab::SetupCollection ()
	{
		Ui_.CollectionTree_->setItemDelegate (new CollectionDelegate (Ui_.CollectionTree_));
		auto collection = Core::Instance ().GetLocalCollection ();
		Ui_.CollectionTree_->setModel (collection->GetCollectionModel ());

		QAction *addToPlaylist = new QAction (tr ("Add to playlist"), this);
		addToPlaylist->setProperty ("ActionIcon", "list-add");
		connect (addToPlaylist,
				SIGNAL (triggered ()),
				this,
				SLOT (loadFromCollection ()));
		Ui_.CollectionTree_->addAction (addToPlaylist);
	}

	void PlayerTab::SetupFSBrowser ()
	{
		FSModel_->setReadOnly (true);
		FSModel_->setRootPath (QDir::rootPath ());
		Ui_.FSTree_->setModel (FSModel_);

		QAction *addToPlaylist = new QAction (tr ("Add to playlist"), this);
		addToPlaylist->setProperty ("ActionIcon", "list-add");
		connect (addToPlaylist,
				SIGNAL (triggered ()),
				this,
				SLOT (loadFromFSBrowser ()));
		Ui_.FSTree_->addAction (addToPlaylist);
	}

	void PlayerTab::SetupPlaylist ()
	{
		Ui_.Playlist_->setItemDelegate (new PlaylistDelegate (Ui_.Playlist_, Ui_.Playlist_));
		Ui_.Playlist_->setModel (Player_->GetPlaylistModel ());
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
		auto playGroup = new QActionGroup (this);
		for (size_t i = 0; i < modes.size (); ++i)
		{
			QAction *action = new QAction (names [i], this);
			action->setProperty ("PlayMode", static_cast<int> (modes.at (i)));
			action->setCheckable (true);
			action->setChecked (modes.at (i) == Player::PlayMode::Sequential);
			action->setActionGroup (playGroup);
			playMode->addAction (action);

			connect (action,
					SIGNAL (triggered ()),
					this,
					SLOT (handleChangePlayMode ()));
		}

		PlaylistToolbar_->addWidget (playButton);
	}

	void PlayerTab::handleSongChanged (const MediaInfo& info)
	{
		QPixmap px;
		if (info.LocalPath_.isEmpty ())
			px = QIcon::fromTheme ("media-optical").pixmap (128, 128);
		else
		{
			px = FindAlbumArt (info.LocalPath_);
			if (px.isNull ())
				px = QIcon::fromTheme ("media-optical").pixmap (128, 128);
		}
		Ui_.NPWidget_->SetAlbumArt (px);
		const QPixmap& scaled = px.scaled (Ui_.NPArt_->minimumSize (),
				Qt::KeepAspectRatio, Qt::SmoothTransformation);
		Ui_.NPArt_->setPixmap (scaled);

		Ui_.NPWidget_->SetTrackInfo (info);

		Ui_.NowPlaying_->clear ();
		if (!info.Title_.isEmpty () || !info.Artist_.isEmpty ())
		{
			const auto& title = info.Title_.isEmpty () ? tr ("unknown song") : info.Title_;
			const auto& album = info.Album_.isEmpty () ? tr ("unknown album") : info.Album_;
			const auto& track = info.Artist_.isEmpty () ? tr ("unknown artist") : info.Artist_;

			const QString& text = tr ("Now playing: %1 from %2 by %3")
					.arg ("<em>" + title + "</em>")
					.arg ("<em>" + album + "</em>")
					.arg ("<em>" + track + "</em>");
			Ui_.NowPlaying_->setText (text);

			if (XmlSettingsManager::Instance ().property ("EnableNotifications").toBool ())
			{
				Entity e = Util::MakeNotification ("LMP", text, PInfo_);
				e.Additional_ ["NotificationPixmap"] = px;
				emit gotEntity (e);
			}
		}
	}

	void PlayerTab::handleScanProgress (int progress)
	{
		if (progress >= Ui_.ScanProgress_->maximum ())
		{
			Ui_.ScanProgress_->hide ();
			return;
		}

		if (!Ui_.ScanProgress_->isVisible ())
			Ui_.ScanProgress_->show ();
		Ui_.ScanProgress_->setValue (progress);
	}

	void PlayerTab::handleChangePlayMode ()
	{
		auto mode = sender ()->property ("PlayMode").toInt ();
		Player_->SetPlayMode (static_cast<Player::PlayMode> (mode));
	}

	void PlayerTab::loadFromCollection ()
	{
		const QModelIndex& index = Ui_.CollectionTree_->currentIndex ();
		if (!index.isValid ())
			return;

		auto collection = Core::Instance ().GetLocalCollection ();
		collection->Enqueue (index, Player_);
	}

	void PlayerTab::loadFromFSBrowser ()
	{
		const QModelIndex& index = Ui_.FSTree_->currentIndex ();
		if (!index.isValid ())
			return;

		const QFileInfo& fi = FSModel_->fileInfo (index);

		if (fi.isDir ())
			Player_->Enqueue (RecIterate (fi.absoluteFilePath ()));
		else
			Player_->Enqueue (QStringList (fi.absoluteFilePath ()));
	}

	void PlayerTab::loadFromDisk ()
	{
		QStringList files = QFileDialog::getOpenFileNames (this,
				tr ("Load files"),
				QDir::homePath (),
				tr ("Music files (*.ogg *.flac *.mp3 *.wav);;All files (*.*)"));
		Player_->Enqueue (files);
	}
}
}
