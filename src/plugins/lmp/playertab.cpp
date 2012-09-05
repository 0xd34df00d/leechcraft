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
#include <QMenu>
#include <QDialogButtonBox>
#include <QListWidget>
#include <QTabBar>
#include <QMessageBox>
#include <phonon/seekslider.h>
#include <util/util.h>
#include <util/gui/clearlineeditaddon.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/media/iaudioscrobbler.h>
#include <interfaces/media/isimilarartists.h>
#include <interfaces/media/ipendingsimilarartists.h>
#include <interfaces/media/ilyricsfinder.h>
#include <interfaces/core/icoreproxy.h>
#include "player.h"
#include "util.h"
#include "core.h"
#include "localcollection.h"
#include "collectiondelegate.h"
#include "xmlsettingsmanager.h"
#include "aalabeleventfilter.h"
#include "nowplayingpixmaphandler.h"

#ifdef ENABLE_MPRIS
#include "mpris/instance.h"
#endif

Q_DECLARE_METATYPE (Phonon::MediaSource);

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
	, TabToolbar_ (new QToolBar ())
	, PlayPause_ (0)
	, TrayMenu_ (new QMenu ("LMP", this))
	, NPPixmapHandler_ (new NowPlayingPixmapHandler (this))
	{
		Ui_.setupUi (this);
		Ui_.MainSplitter_->setStretchFactor (0, 2);
		Ui_.MainSplitter_->setStretchFactor (1, 1);
		Ui_.RadioWidget_->SetPlayer (Player_);

		NPPixmapHandler_->AddSetter ([this] (const QPixmap& px, const QString&) { Ui_.NPWidget_->SetAlbumArt (px); });
		NPPixmapHandler_->AddSetter ([this] (const QPixmap& px, const QString& path)
				{
					const QPixmap& scaled = px.scaled (Ui_.NPArt_->minimumSize (),
							Qt::KeepAspectRatio, Qt::SmoothTransformation);
					Ui_.NPArt_->setPixmap (scaled);
					Ui_.NPArt_->setProperty ("LMP/CoverPath", path);
				});

		new Util::ClearLineEditAddon (Core::Instance ().GetProxy (), Ui_.CollectionFilter_);

		SetupNavButtons ();

		Ui_.FSBrowser_->AssociatePlayer (Player_);

		auto coverGetter = [this] () { return Ui_.NPArt_->property ("LMP/CoverPath").toString (); };
		Ui_.NPArt_->installEventFilter (new AALabelEventFilter (coverGetter, this));

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
		connect (Core::Instance ().GetLocalCollection (),
				SIGNAL (scanFinished ()),
				Ui_.ScanProgress_,
				SLOT (hide ()));
		Ui_.ScanProgress_->hide ();

		TrayIcon_ = new LMPSystemTrayIcon (QIcon (":/lmp/resources/images/lmp.svg"), this);
		connect (Player_,
				SIGNAL (songChanged (const MediaInfo&)),
				TrayIcon_,
				SLOT (handleSongChanged (const MediaInfo&)));
		SetupToolbar ();
		SetupCollection ();
		Ui_.PLManagerWidget_->SetPlayer (Player_);

		Ui_.Playlist_->SetPlayer (Player_);

		XmlSettingsManager::Instance ().RegisterObject ("ShowTrayIcon",
				this, "handleShowTrayIcon");
		handleShowTrayIcon ();

		XmlSettingsManager::Instance ().RegisterObject ("UseNavTabBar",
				this, "handleUseNavTabBar");
		handleUseNavTabBar ();

		connect (Ui_.NPWidget_,
				SIGNAL (gotArtistImage (QString, QUrl)),
				NPPixmapHandler_,
				SLOT (handleGotArtistImage (QString, QUrl)));

#ifdef ENABLE_MPRIS
		new MPRIS::Instance (this, Player_);
#endif
	}

	PlayerTab::~PlayerTab ()
	{
		delete NavBar_;
		delete NavButtons_;
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

	void PlayerTab::InitWithOtherPlugins ()
	{
		handleSongChanged (MediaInfo ());
		Ui_.DevicesBrowser_->InitializeDevices ();
		Ui_.RadioWidget_->InitializeProviders ();
	}

	void PlayerTab::SetupNavButtons ()
	{
		NavBar_ = new QTabBar ();
		NavBar_->setShape (QTabBar::RoundedWest);
		NavBar_->hide ();

		NavButtons_ = new QListWidget ();
		NavButtons_->hide ();
		NavButtons_->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Expanding);
		NavButtons_->setFixedWidth (70);
		NavButtons_->setStyleSheet ("background-color: palette(window);");
		NavButtons_->setVerticalScrollBarPolicy (Qt::ScrollBarAlwaysOff);
		NavButtons_->setFrameShape (QFrame::NoFrame);
		NavButtons_->setFrameShadow (QFrame::Plain);
		NavButtons_->setIconSize (QSize (24, 24));
		NavButtons_->setViewMode (QListView::IconMode);
		NavButtons_->setWordWrap (true);
		NavButtons_->setGridSize (QSize (70, 65));
		NavButtons_->setMovement (QListView::Static);
		NavButtons_->setFlow (QListView::TopToBottom);

		auto mkButton = [this] (const QString& title, const QString& iconName)
		{
			const auto& icon = Core::Instance ().GetProxy ()->GetIcon (iconName);

			auto but = new QListWidgetItem (title);
			NavButtons_->addItem (but);
			but->setToolTip (title);
			but->setSizeHint (NavButtons_->gridSize ());
			but->setTextAlignment (Qt::AlignCenter);
			but->setIcon (icon);

			NavBar_->addTab (icon, title);
		};

		mkButton (tr ("Current song"), "view-media-lyrics");
		mkButton (tr ("Collection"), "folder-sound");
		mkButton (tr ("Playlists"), "view-media-playlist");
		mkButton (tr ("Social"), "system-users");
		mkButton (tr ("Internet"), "applications-internet");
		mkButton (tr ("Filesystem"), "document-open");
		mkButton (tr ("Devices"), "drive-removable-media-usb");

		NavButtons_->setCurrentRow (0);

		connect (NavBar_,
				SIGNAL (currentChanged (int)),
				Ui_.WidgetsStack_,
				SLOT (setCurrentIndex (int)));
		connect (NavButtons_,
				SIGNAL (currentRowChanged (int)),
				Ui_.WidgetsStack_,
				SLOT (setCurrentIndex (int)));
		connect (Ui_.WidgetsStack_,
				SIGNAL (currentChanged (int)),
				NavBar_,
				SLOT (setCurrentIndex (int)));
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
		TrayMenu_->addAction (love);
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

		CollectionShowTrackProps_ = new QAction (tr ("Show track properties"), Ui_.CollectionTree_);
		CollectionShowTrackProps_->setProperty ("ActionIcon", "document-properties");
		connect (CollectionShowTrackProps_,
				SIGNAL (triggered ()),
				this,
				SLOT (showCollectionTrackProps ()));
		Ui_.CollectionTree_->addAction (CollectionShowTrackProps_);

		Ui_.CollectionTree_->addAction (Util::CreateSeparator (Ui_.CollectionTree_));

		CollectionRemove_ = new QAction (tr ("Remove from collection..."), Ui_.CollectionTree_);
		CollectionRemove_->setProperty ("ActionIcon", "list-remove");
		connect (CollectionRemove_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleCollectionRemove ()));
		Ui_.CollectionTree_->addAction (CollectionRemove_);

		CollectionDelete_ = new QAction (tr ("Delete from disk..."), Ui_.CollectionTree_);
		CollectionDelete_->setProperty ("ActionIcon", "edit-delete");
		connect (CollectionDelete_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleCollectionDelete ()));
		Ui_.CollectionTree_->addAction (CollectionDelete_);

		connect (Ui_.CollectionTree_,
				SIGNAL (doubleClicked (QModelIndex)),
				this,
				SLOT (loadFromCollection ()));

		connect (Ui_.CollectionTree_->selectionModel (),
				SIGNAL (currentRowChanged (QModelIndex, QModelIndex)),
				this,
				SLOT (handleCollectionItemSelected (QModelIndex)));

		connect (Ui_.CollectionFilter_,
				SIGNAL (textChanged (QString)),
				CollectionFilterModel_,
				SLOT (setFilterFixedString (QString)));
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
				QPixmap notifyPx = px;
				int width = notifyPx.width ();
				if (width > 200)
				{
					while (width > 200)
						width /= 2;
					notifyPx = notifyPx.scaledToWidth (width);
				}

				Entity e = Util::MakeNotification ("LMP", text, PInfo_);
				e.Additional_ ["NotificationPixmap"] = notifyPx;
				emit gotEntity (e);
			}
		}
	}

	void PlayerTab::Scrobble (const MediaInfo& info)
	{
		if (!XmlSettingsManager::Instance ()
				.property ("EnableScrobbling").toBool ())
			return;

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
		auto coverPath = FindAlbumArtPath (info.LocalPath_);
		QPixmap px;
		if (!coverPath.isEmpty ())
			px = QPixmap (coverPath);

		const bool isCorrect = !px.isNull ();
		if (!isCorrect)
		{
			px = QIcon::fromTheme ("media-optical").pixmap (128, 128);
			coverPath.clear ();
		}

		NPPixmapHandler_->HandleSongChanged (info, coverPath, px, isCorrect);

		Ui_.NPWidget_->SetTrackInfo (info);

		SetNowPlaying (info, px);
		Scrobble (info);
		RequestLyrics (info);

		if (info.Artist_.isEmpty ())
		{
			LastArtist_.clear ();
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
		else if (info.Artist_ != LastArtist_)
		{
			LastArtist_ = info.Artist_;
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
		if (!XmlSettingsManager::Instance ()
				.property ("EnableScrobbling").toBool ())
			return;

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
		LastArtist_ = obj->GetSourceArtistName ();
		Similars_ [LastArtist_] = similar;
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

	void PlayerTab::showCollectionTrackProps ()
	{
		const auto& index = Ui_.CollectionTree_->currentIndex ();
		const auto& info = index.data (LocalCollection::Role::TrackPath).value<QString> ();
		if (info.isEmpty ())
			return;

		AudioPropsWidget::MakeDialog ()->SetProps (info);
	}

	namespace
	{
		template<typename T>
		QList<T> CollectFromModel (const QModelIndex& root, int role)
		{
			QList<T> result;

			const auto& var = root.data (role);
			if (!var.isNull ())
				result << var.value<T> ();

			auto model = root.model ();
			for (int i = 0; i < model->rowCount (root); ++i)
				result += CollectFromModel<T> (root.child (i, 0), role);

			return result;
		}
	}

	void PlayerTab::handleCollectionRemove ()
	{
		const auto& index = Ui_.CollectionTree_->currentIndex ();
		const auto& paths = CollectFromModel<QString> (index, LocalCollection::Role::TrackPath);
		if (paths.isEmpty ())
			return;

		auto response = QMessageBox::question (this,
				"LeechCraft",
				tr ("Are you sure you want to remove %n track(s) from your collection?<br/><br/>"
					"Please note that if tracks remain on your disk they will be re-added next "
					"time collection is scanned, but you will lose the statistics.",
					0,
					paths.size ()),
					QMessageBox::Yes | QMessageBox::No);
		if (response != QMessageBox::Yes)
			return;

		auto collection = Core::Instance ().GetLocalCollection ();
		Q_FOREACH (const auto& path, paths)
			collection->RemoveTrack (path);
	}

	void PlayerTab::handleCollectionDelete ()
	{
		const auto& index = Ui_.CollectionTree_->currentIndex ();
		const auto& paths = CollectFromModel<QString> (index, LocalCollection::Role::TrackPath);
		if (paths.isEmpty ())
			return;

		auto response = QMessageBox::question (this,
				"LeechCraft",
				tr ("Are you sure you want to erase %n track(s)? This action cannot be undone.",
					0,
					paths.size ()),
					QMessageBox::Yes | QMessageBox::No);
		if (response != QMessageBox::Yes)
			return;

		Q_FOREACH (const auto& path, paths)
			QFile::remove (path);
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

	void PlayerTab::handleCollectionItemSelected (const QModelIndex& index)
	{
		const int nodeType = index.data (LocalCollection::Role::Node).value<int> ();
		CollectionShowTrackProps_->setEnabled (nodeType == LocalCollection::NodeType::Track);
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

	void PlayerTab::handleUseNavTabBar ()
	{
		if (Ui_.WidgetsLayout_->count () == 2)
		{
			auto item = Ui_.WidgetsLayout_->takeAt (0);
			item->widget ()->hide ();
			delete item;
		}
		const bool useTabs = XmlSettingsManager::Instance ().property ("UseNavTabBar").toBool ();
		QWidget *widget = useTabs ?
				static_cast<QWidget*> (NavBar_) :
				static_cast<QWidget*> (NavButtons_);
		Ui_.WidgetsLayout_->insertWidget (0, widget, 0,
				useTabs ? Qt::AlignTop : Qt::Alignment ());
		widget->show ();
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
