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
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QToolButton>
#include <QMenu>
#include <QInputDialog>
#include <phonon/seekslider.h>
#include <util/util.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/media/iaudioscrobbler.h>
#include <interfaces/media/isimilarartists.h>
#include <interfaces/media/ipendingsimilarartists.h>
#include <interfaces/media/ilyricsfinder.h>
#include <interfaces/core/icoreproxy.h>
#include "player.h"
#include "playlistdelegate.h"
#include "util.h"
#include "core.h"
#include "localcollection.h"
#include "collectiondelegate.h"
#include "xmlsettingsmanager.h"
#include "playlistmanager.h"
#include "staticplaylistmanager.h"

namespace LeechCraft
{
namespace LMP
{
	namespace
	{
		class CollectionFilterModel : public QSortFilterProxyModel
		{
		public:
			CollectionFilterModel (QObject *parent = 0)
			: QSortFilterProxyModel (parent)
			{
				setDynamicSortFilter (true);
			}
		protected:
			bool filterAcceptsRow (int sourceRow, const QModelIndex& sourceParent) const
			{
				const auto& source = sourceModel ()->index (sourceRow, 0, sourceParent);
				const auto type = source.data (LocalCollection::Role::Node).toInt ();

				const auto& pattern = filterRegExp ().pattern ();

				if (type == LocalCollection::NodeType::Artist ||
					type == LocalCollection::NodeType::Album)
					for (int i = 0, rc = sourceModel ()->rowCount (source); i < rc; ++i)
						if (filterAcceptsRow (i, source))
							return true;

				return source.data ().toString ().contains (pattern, Qt::CaseInsensitive);
			}
		};
	}

	PlayerTab::PlayerTab (const TabClassInfo& info, QObject *plugin, QWidget *parent)
	: QWidget (parent)
	, Plugin_ (plugin)
	, TC_ (info)
	, CollectionFilterModel_ (new CollectionFilterModel (this))
	, Player_ (new Player (this))
	, PlaylistToolbar_ (new QToolBar ())
	, TabToolbar_ (new QToolBar ())
	, PlayPause_ (0)
	, TrayMenu_ (new QMenu ("LMP", this))
	{
		Ui_.setupUi (this);
		Ui_.MainSplitter_->setStretchFactor (0, 2);
		Ui_.MainSplitter_->setStretchFactor (1, 1);
		Ui_.RadioWidget_->SetPlayer (Player_);

		Ui_.FSBrowser_->AssociatePlayer (Player_);

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

		TrayIcon_ = new LMPSystemTrayIcon (QIcon (":/lmp/resources/images/lmp.svg"), this);
		connect (Player_,
				SIGNAL (songChanged (const MediaInfo&)),
				TrayIcon_,
				SLOT (handleSongChanged (const MediaInfo&)));
		SetupToolbar ();
		SetupCollection ();
		SetupPlaylistsTab ();
		SetupPlaylist ();

		XmlSettingsManager::Instance ().RegisterObject ("ShowTrayIcon",
				this, "handleShowTrayIcon");
		handleShowTrayIcon ();
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

	Player* PlayerTab::GetPlayer () const
	{
		return Player_;
	}

	QByteArray PlayerTab::GetTabRecoverData () const
	{
		return "playertab";
	}

	QIcon PlayerTab::GetTabRecoverIcon () const
	{
		return QIcon (":/lmp/resources/images/lmp.svg");
	}

	QString PlayerTab::GetTabRecoverName () const
	{
		return "LMP";
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

		PlayPause_ = new QAction (tr ("Play/Pause"), this);
		PlayPause_->setProperty ("ActionIcon", "media-playback-start");
		PlayPause_->setProperty ("WatchActionIconChange", true);
		connect (PlayPause_,
				SIGNAL (triggered ()),
				Player_,
				SLOT (togglePause ()));
		TabToolbar_->addAction (PlayPause_);

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

		QAction *love = new QAction (tr ("Love"), this);
		love->setProperty ("ActionIcon", "emblem-favorite");
		love->setShortcut (QString ("Ctrl+L"));
		connect (love,
				SIGNAL (triggered ()),
				this,
				SLOT (handleLoveTrack ()));
		TabToolbar_->addAction (love);

		TabToolbar_->addSeparator ();

		PlayedTime_ = new QLabel ();
		RemainingTime_ = new QLabel ();
		auto seekSlider = new Phonon::SeekSlider (Player_->GetSourceObject ());
		seekSlider->setTracking (false);
		TabToolbar_->addWidget (PlayedTime_);
		TabToolbar_->addWidget (seekSlider);
		TabToolbar_->addWidget (RemainingTime_);
		TabToolbar_->addSeparator ();
		connect (Player_->GetSourceObject (),
				SIGNAL (tick (qint64)),
				this,
				SLOT (handleCurrentPlayTime (qint64)));

		auto volumeSlider = new Phonon::VolumeSlider (Player_->GetAudioOutput ());
		volumeSlider->setMinimumWidth (100);
		volumeSlider->setMaximumWidth (160);
		TabToolbar_->addWidget (volumeSlider);

		// fill tray menu
		connect (TrayIcon_,
				SIGNAL (changedVolume (qreal)),
				this,
				SLOT (handleChangedVolume (qreal)));
		connect (TrayIcon_,
				SIGNAL (activated (QSystemTrayIcon::ActivationReason)),
				this,
				SLOT (handleTrayIconActivated (QSystemTrayIcon::ActivationReason)));

		QAction *closeLMP = new QAction (tr ("Close LMP"), TrayIcon_);
		closeLMP->setProperty ("ActionIcon", "edit-delete");
		connect (closeLMP,
				SIGNAL (triggered ()),
				this,
				SLOT (closeLMP ()));

		connect (Player_->GetSourceObject (),
				SIGNAL (stateChanged (Phonon::State, Phonon::State)),
				this,
				SLOT (handleStateChanged (Phonon::State, Phonon::State)));
		TrayMenu_->addAction (previous);
		TrayMenu_->addAction (PlayPause_);
		TrayMenu_->addAction (stop);
		TrayMenu_->addAction (next);
		TrayMenu_->addSeparator ();
		TrayMenu_->addAction (closeLMP);
		TrayIcon_->setContextMenu (TrayMenu_);
	}

	void PlayerTab::SetupCollection ()
	{
		Ui_.CollectionTree_->setItemDelegate (new CollectionDelegate (Ui_.CollectionTree_));
		auto collection = Core::Instance ().GetLocalCollection ();
		CollectionFilterModel_->setSourceModel (collection->GetCollectionModel ());
		Ui_.CollectionTree_->setModel (CollectionFilterModel_);

		QAction *addToPlaylist = new QAction (tr ("Add to playlist"), this);
		addToPlaylist->setProperty ("ActionIcon", "list-add");
		connect (addToPlaylist,
				SIGNAL (triggered ()),
				this,
				SLOT (loadFromCollection ()));
		Ui_.CollectionTree_->addAction (addToPlaylist);
		connect (Ui_.CollectionTree_,
				SIGNAL (doubleClicked (QModelIndex)),
				this,
				SLOT (loadFromCollection ()));

		connect (Ui_.CollectionFilter_,
				SIGNAL (textChanged (QString)),
				CollectionFilterModel_,
				SLOT (setFilterFixedString (QString)));
	}

	void PlayerTab::SetupPlaylistsTab ()
	{
		auto mgr = Core::Instance ().GetPlaylistManager ();
		Ui_.PlaylistsTree_->setModel (mgr->GetPlaylistsModel ());
		Ui_.PlaylistsTree_->expandAll ();

		connect (Ui_.PlaylistsTree_,
				SIGNAL (doubleClicked (QModelIndex)),
				this,
				SLOT (handlePlaylistSelected (QModelIndex)));
	}

	void PlayerTab::SetupPlaylist ()
	{
		Ui_.Playlist_->setItemDelegate (new PlaylistDelegate (Ui_.Playlist_, Ui_.Playlist_));
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

		const int resumeMode = XmlSettingsManager::Instance ()
				.Property ("PlayMode", static_cast<int> (Player::PlayMode::Sequential)).toInt ();
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
			action->setChecked (static_cast<int> (modes.at (i)) == resumeMode);
			action->setActionGroup (playGroup);
			playMode->addAction (action);

			connect (action,
					SIGNAL (triggered ()),
					this,
					SLOT (handleChangePlayMode ()));
		}
		Player_->SetPlayMode (static_cast<Player::PlayMode> (resumeMode));

		PlaylistToolbar_->addWidget (playButton);

		auto removeSelected = new QAction (tr ("Delete from playlist"), Ui_.Playlist_);
		removeSelected->setProperty ("ActionIcon", "list-remove");
		removeSelected->setShortcut (Qt::Key_Delete);
		connect (removeSelected,
				SIGNAL (triggered ()),
				this,
				SLOT (removeSelectedSongs ()));
		Ui_.Playlist_->addAction (removeSelected);

		Ui_.Playlist_->addAction (Util::CreateSeparator (this));

		auto stopAfterSelected = new QAction (tr ("Stop after this track"), Ui_.Playlist_);
		stopAfterSelected->setProperty ("ActionIcon", "media-playback-stop");
		connect (stopAfterSelected,
				SIGNAL (triggered ()),
				this,
				SLOT (setStopAfterSelected ()));
		Ui_.Playlist_->addAction (stopAfterSelected);
	}

	void PlayerTab::SetNowPlaying (const MediaInfo& info, const QPixmap& px)
	{
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

	void PlayerTab::Scrobble (const MediaInfo& info)
	{
		auto scrobblers = Core::Instance ().GetProxy ()->
					GetPluginsManager ()->GetAllCastableTo<Media::IAudioScrobbler*> ();
		if (info.Title_.isEmpty () && info.Artist_.isEmpty ())
		{
			std::for_each (scrobblers.begin (), scrobblers.end (),
					[] (decltype (scrobblers.front ()) s) { s->PlaybackStopped (); });
			return;
		}

		const Media::AudioInfo aInfo = info;
		std::for_each (scrobblers.begin (), scrobblers.end (),
					[&aInfo] (decltype (scrobblers.front ()) s) { s->NowPlaying (aInfo); });
	}

	void PlayerTab::FillSimilar (const Media::SimilarityInfos_t& infos)
	{
		Ui_.NPWidget_->SetSimilarArtists (infos);
	}

	void PlayerTab::RequestLyrics (const MediaInfo& info)
	{
		Ui_.NPWidget_->SetLyrics (QString ());

		if (!XmlSettingsManager::Instance ().property ("RequestLyrics").toBool ())
			return;

		auto finders = Core::Instance ().GetProxy ()->
					GetPluginsManager ()->GetAllCastableRoots<Media::ILyricsFinder*> ();
		Q_FOREACH (auto finderObj, finders)
		{
			connect (finderObj,
					SIGNAL (gotLyrics (const Media::LyricsQuery&, const QStringList&)),
					this,
					SLOT (handleGotLyrics (const Media::LyricsQuery&, const QStringList&)),
					Qt::UniqueConnection);
			auto finder = qobject_cast<Media::ILyricsFinder*> (finderObj);
			finder->RequestLyrics ({ info.Artist_, info.Album_, info.Title_ },
					Media::QueryOption::NoOption);
		}
	}

	namespace
	{
		QIcon GetIconFromState (Phonon::State state)
		{
			switch (state)
			{
				case Phonon::PlayingState:
					return Core::Instance ().GetProxy ()->GetIcon ("media-playback-start");
				case Phonon::PausedState:
					return Core::Instance ().GetProxy ()->GetIcon ("media-playback-pause");
				default:
					return QIcon ();
			}
		}

		template<typename T>
		void UpdateIcon (T iconable, Phonon::State state,
				std::function<QSize (T)> iconSizeGetter)
		{
			QIcon icon = GetIconFromState (state);
			QIcon baseIcon = icon.isNull() ?
				QIcon (":/lmp/resources/images/lmp.svg") :
				iconable->icon ();

			const QSize& iconSize = iconSizeGetter (iconable);

			QPixmap px = baseIcon.pixmap (iconSize);

			if (!icon.isNull ())
			{
				QPixmap statePx = icon.pixmap (iconSize);

				QPainter p (&px);
				p.drawPixmap (0 + iconSize.width () / 2,
						0 + iconSize.height () / 2 ,
						iconSize.width () / 2,
						iconSize.height () / 2,
						statePx);
				p.end ();
			}

			iconable->setIcon (QIcon (px));
		}
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

		SetNowPlaying (info, px);
		Scrobble (info);
		RequestLyrics (info);

		if (info.Artist_.isEmpty ())
		{
			LastSimilar_.clear ();
			FillSimilar (Media::SimilarityInfos_t ());
		}
		else if (!Similars_.contains (info.Artist_))
		{
			auto similars = Core::Instance ().GetProxy ()->
					GetPluginsManager ()->GetAllCastableTo<Media::ISimilarArtists*> ();
			qDebug () << Q_FUNC_INFO << similars.size ();
			Q_FOREACH (auto *similar, similars)
			{
				auto obj = similar->GetSimilarArtists (info.Artist_, 15);
				if (!obj)
					continue;
				connect (obj->GetObject (),
						SIGNAL (error ()),
						this,
						SLOT (handleSimilarError ()));
				connect (obj->GetObject (),
						SIGNAL (ready ()),
						this,
						SLOT (handleSimilarReady ()));
			}
		}
		else if (info.Artist_ != LastSimilar_)
		{
			LastSimilar_ = info.Artist_;
			FillSimilar (Similars_ [info.Artist_]);
		}
	}

	void PlayerTab::handleCurrentPlayTime (qint64 time)
	{
		auto niceTime = [] (qint64 time)
		{
			if (!time)
				return QString ();

			QString played = Util::MakeTimeFromLong (time / 1000);
			if (played.startsWith ("00:"))
				played = played.mid (3);
			return played;
		};
		PlayedTime_->setText (niceTime (time));

		const auto remaining = Player_->GetSourceObject ()->remainingTime ();
		RemainingTime_->setText (remaining < 0 ? tr ("unknown") : niceTime (remaining));
	}

	void PlayerTab::handleLoveTrack ()
	{
		auto scrobblers = Core::Instance ().GetProxy ()->
					GetPluginsManager ()->GetAllCastableTo<Media::IAudioScrobbler*> ();
		std::for_each (scrobblers.begin (), scrobblers.end (),
				[] (decltype (scrobblers.front ()) s) { s->LoveCurrentTrack (); });
	}

	void PlayerTab::handleSimilarError ()
	{
		qWarning () << Q_FUNC_INFO;
		sender ()->deleteLater ();
	}

	void PlayerTab::handleSimilarReady ()
	{
		sender ()->deleteLater ();
		auto obj = qobject_cast<Media::IPendingSimilarArtists*> (sender ());

		const auto& similar = obj->GetSimilar ();
		LastSimilar_ = obj->GetSourceArtistName ();
		Similars_ [LastSimilar_] = similar;
		FillSimilar (similar);
	}

	void PlayerTab::handleGotLyrics (const Media::LyricsQuery&, const QStringList& lyrics)
	{
		if (lyrics.isEmpty ())
			return;

		Ui_.NPWidget_->SetLyrics (lyrics.value (0));
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
		XmlSettingsManager::Instance ().setProperty ("PlayMode", mode);
	}

	void PlayerTab::handlePlaylistSelected (const QModelIndex& index)
	{
		auto mgr = Core::Instance ().GetPlaylistManager ();
		const auto& sources = mgr->GetSources (index);
		if (sources.isEmpty ())
			return;

		Player_->Enqueue (sources, false);
	}

	void PlayerTab::removeSelectedSongs ()
	{
		auto selModel = Ui_.Playlist_->selectionModel ();
		if (!selModel)
			return;

		auto indexes = selModel->selectedRows ();
		if (indexes.isEmpty ())
			indexes << Ui_.Playlist_->currentIndex ();
		indexes.removeAll (QModelIndex ());

		QList<QPersistentModelIndex> persistent;
		std::copy (indexes.begin (), indexes.end (), std::back_inserter (persistent));
		Q_FOREACH (const auto& idx, persistent)
			if (idx.isValid ())
				Player_->Dequeue (idx);
	}

	void PlayerTab::setStopAfterSelected ()
	{
		auto index = Ui_.Playlist_->currentIndex ();
		if (!index.isValid ())
			return;

		Player_->SetStopAfter (index);
	}

	void PlayerTab::loadFromCollection ()
	{
		const auto& idxs = Ui_.CollectionTree_->selectionModel ()->selectedRows ();
		auto collection = Core::Instance ().GetLocalCollection ();
		Q_FOREACH (const auto& src, idxs)
		{
			const QModelIndex& index = CollectionFilterModel_->mapToSource (src);
			if (!index.isValid ())
				continue;
			collection->Enqueue (index, Player_);
		}
	}

	void PlayerTab::handleSavePlaylist ()
	{
		const auto& name = QInputDialog::getText (this,
				tr ("Save playlist"),
				tr ("Enter name for the playlist:"));
		if (name.isEmpty ())
			return;

		auto mgr = Core::Instance ().GetPlaylistManager ()->GetStaticManager ();
		mgr->SaveCustomPlaylist (name, Player_->GetQueue ());
	}

	void PlayerTab::loadFromDisk ()
	{
		QStringList files = QFileDialog::getOpenFileNames (this,
				tr ("Load files"),
				QDir::homePath (),
				tr ("Music files (*.ogg *.flac *.mp3 *.wav);;All files (*.*)"));
		Player_->Enqueue (files);
	}

	void PlayerTab::closeLMP ()
	{
		Remove ();
	}

	void PlayerTab::handleStateChanged (Phonon::State newState, Phonon::State)
	{
		if (newState == Phonon::PlayingState)
			PlayPause_->setProperty ("ActionIcon", "media-playback-pause");
		else
		{
			if (newState == Phonon::StoppedState)
				TrayIcon_->handleSongChanged (MediaInfo ());
			PlayPause_->setProperty ("ActionIcon", "media-playback-start");
		}
		UpdateIcon<LMPSystemTrayIcon*> (TrayIcon_, newState,
				[] (QSystemTrayIcon *icon) { return icon->geometry ().size (); });
	}

	void PlayerTab::handleShowTrayIcon ()
	{
		TrayIcon_->setVisible (XmlSettingsManager::Instance ().property ("ShowTrayIcon").toBool ());
	}

	void PlayerTab::handleChangedVolume (qreal delta)
	{
		qreal volume = Player_->GetAudioOutput ()->volume ();
		qreal dl = delta > 0 ? 0.05 : -0.05;
		if (volume != volume)
			volume = 0.0;

		volume += dl;
		if (volume < 0)
			volume = 0;
		else if (volume > 1)
			volume = 1;

		Player_->GetAudioOutput ()->setVolume (volume);
	}

	void PlayerTab::handleTrayIconActivated (QSystemTrayIcon::ActivationReason reason)
	{
		switch (reason)
		{
			case QSystemTrayIcon::MiddleClick:
				Player_->togglePause ();
				break;
			default:
				break;
		}
	}

}
}
