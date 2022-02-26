/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "playertab.h"
#include <algorithm>
#include <functional>
#include <QToolBar>
#include <QFileDialog>
#include <QStandardItemModel>
#include <QMenu>
#include <QDialogButtonBox>
#include <QListWidget>
#include <QTabBar>
#include <QMessageBox>
#include <QToolButton>
#include <QPainter>
#include <util/gui/util.h>
#include <util/xpc/util.h>
#include <util/sll/slotclosure.h>
#include <util/sll/visitor.h>
#include <util/sll/either.h>
#include <util/threads/futures.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/media/iaudioscrobbler.h>
#include <interfaces/media/isimilarartists.h>
#include <interfaces/media/ilyricsfinder.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>

#ifdef ENABLE_MPRIS
#include "mpris/instance.h"
#endif

#include "player.h"
#include "util.h"
#include "core.h"
#include "localcollection.h"
#include "xmlsettingsmanager.h"
#include "aalabeleventfilter.h"
#include "nowplayingpixmaphandler.h"
#include "engine/sourceobject.h"
#include "engine/output.h"
#include "volumeslider.h"
#include "seekslider.h"
#include "palettefixerfilter.h"
#include "npstateupdater.h"
#include "nptooltiphook.h"

namespace LC
{
namespace LMP
{
	PlayerTab::PlayerTab (const TabClassInfo& info, Player *player, const ICoreProxy_ptr& proxy, QObject *plugin, QWidget *parent)
	: QWidget (parent)
	, Plugin_ (plugin)
	, TC_ (info)
	, Player_ (player)
	, TabToolbar_ (new QToolBar ())
	, PlayPause_ (0)
	, TrayMenu_ (new QMenu ("LMP", this))
	, NPPixmapHandler_ (new NowPlayingPixmapHandler (this))
	, EffectsMenu_ (new QMenu (tr ("Effects"), this))
	{
		Ui_.setupUi (this);
		Ui_.MainSplitter_->setStretchFactor (0, 2);
		Ui_.MainSplitter_->setStretchFactor (1, 1);
		Ui_.RadioWidget_->SetPlayer (Player_);

		NPPixmapHandler_->AddSetter ([this] (const QPixmap& px, const QString& path)
				{
					const QPixmap& scaled = px.scaled (Ui_.NPArt_->minimumSize (),
							Qt::KeepAspectRatio, Qt::SmoothTransformation);
					Ui_.NPArt_->setPixmap (scaled);
					Ui_.NPArt_->setProperty ("LMP/CoverPath", path);
				});

		const auto npTooltipHook = new NPTooltipHook { NPPixmapHandler_, this };
		Ui_.NPArt_->installEventFilter (npTooltipHook);
		Ui_.NowPlaying_->installEventFilter (npTooltipHook);

		SetupNavButtons ();

		Ui_.FSBrowser_->AssociatePlayer (Player_);

		auto coverGetter = [this] () { return Ui_.NPArt_->property ("LMP/CoverPath").toString (); };
		Ui_.NPArt_->installEventFilter (new AALabelEventFilter (coverGetter, this));

		const auto updater = new NPStateUpdater { Ui_.NowPlaying_, Ui_.NPWidget_, Player_, this };
		updater->AddPixmapHandler ([this] (const MediaInfo& info, const QString& path, const QPixmap& px)
					{ NPPixmapHandler_->HandleSongChanged (info, path, px, !px.isNull ()); });
		updater->AddPixmapHandler ([npTooltipHook] (const MediaInfo& info, const QString&, const QPixmap&)
					{ npTooltipHook->SetTrackInfo (info); });
		connect (this,
				SIGNAL (notifyCurrentTrackRequested ()),
				updater,
				SLOT (forceEmitNotification ()));

		connect (Player_,
				SIGNAL (playerAvailable (bool)),
				this,
				SLOT (handlePlayerAvailable (bool)));
		connect (Player_,
				SIGNAL (songChanged (MediaInfo)),
				this,
				SLOT (handleSongChanged (MediaInfo)));
		connect (Player_->GetSourceObject (),
				SIGNAL (stateChanged (SourceState, SourceState)),
				this,
				SLOT (handleStateChanged ()));
		connect (Player_,
				SIGNAL (indexChanged (QModelIndex)),
				Ui_.Playlist_,
				SLOT (focusIndex (QModelIndex)));

		TrayIcon_ = new LMPSystemTrayIcon (Util::FixupTrayIcon (QIcon ("lcicons:/lmp/resources/images/lmp.svg")), this);
		connect (Player_,
				SIGNAL (songChanged (const MediaInfo&)),
				TrayIcon_,
				SLOT (handleSongChanged (const MediaInfo&)));
		SetupToolbar ();
		Ui_.PLManagerWidget_->SetPlayer (Player_);

		Ui_.Playlist_->SetPlayer (Player_, proxy);

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
		emit removeTab ();
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
		return QIcon ("lcicons:/lmp/resources/images/lmp.svg");
	}

	QString PlayerTab::GetTabRecoverName () const
	{
		return "LMP";
	}

	void PlayerTab::AddNPTab (const QString& tabName, QWidget *widget)
	{
		Ui_.NPWidget_->AddTab (tabName, widget);
	}

	void PlayerTab::InitWithOtherPlugins ()
	{
		handleSongChanged (MediaInfo ());
		Ui_.DevicesBrowser_->InitializeDevices ();
		Ui_.EventsWidget_->InitializeProviders ();
		Ui_.ReleasesWidget_->InitializeProviders ();
		Ui_.HypesWidget_->InitializeProviders ();
		Ui_.RecommendationsWidget_->InitializeProviders ();
	}

	void PlayerTab::SetupNavButtons ()
	{
		NavBar_ = new QTabBar ();
		NavBar_->hide ();
		NavBar_->setShape (QTabBar::RoundedWest);
		NavBar_->setUsesScrollButtons (false);
		NavBar_->setElideMode (Qt::ElideRight);
		NavBar_->setExpanding (false);
		NavBar_->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Expanding);

		NavButtons_ = new QListWidget ();
		NavButtons_->hide ();
		NavButtons_->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Expanding);
		NavButtons_->setVerticalScrollBarPolicy (Qt::ScrollBarAlwaysOff);
		NavButtons_->setFrameShape (QFrame::NoFrame);
		NavButtons_->setFrameShadow (QFrame::Plain);

		const QSize iconSize { 48, 48 };

		NavButtons_->setIconSize (iconSize);
		NavButtons_->setGridSize (iconSize + QSize { 8, 8 });
		NavButtons_->setViewMode (QListView::IconMode);
		NavButtons_->setMovement (QListView::Static);
		NavButtons_->setFlow (QListView::TopToBottom);
		new PaletteFixerFilter (NavButtons_);

		NavButtons_->setFixedWidth (NavButtons_->gridSize ().width () + 5);

		const auto itm = GetProxyHolder ()->GetIconThemeManager ();
		auto mkButton = [this, itm] (const QString& title, const QString& iconName)
		{
			const auto& icon = itm->GetIcon (iconName);

			auto but = new QListWidgetItem ();
			NavButtons_->addItem (but);
			but->setToolTip (title);
			but->setSizeHint (NavButtons_->gridSize ());
			but->setTextAlignment (Qt::AlignCenter);
			but->setIcon (icon);

			NavBar_->addTab (icon, title);
			NavBar_->setTabToolTip (NavBar_->count () - 1, title);
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
		const auto previous = new QAction (tr ("Previous track"), this);
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

		const auto stop = new QAction (tr ("Stop"), this);
		stop->setProperty ("ActionIcon", "media-playback-stop");
		connect (stop,
				SIGNAL (triggered ()),
				Player_,
				SLOT (stop ()));
		TabToolbar_->addAction (stop);

		const auto next = new QAction (tr ("Next track"), this);
		next->setProperty ("ActionIcon", "media-skip-forward");
		connect (next,
				SIGNAL (triggered ()),
				Player_,
				SLOT (nextTrack ()));
		TabToolbar_->addAction (next);

		TabToolbar_->addSeparator ();

		const auto love = new QAction (tr ("Love"), this);
		love->setProperty ("ActionIcon", "emblem-favorite");
		love->setShortcut (QString ("Ctrl+L"));
		connect (love,
				SIGNAL (triggered ()),
				this,
				SLOT (handleLoveTrack ()));
		TabToolbar_->addAction (love);

		const auto ban = new QAction (tr ("Ban"), this);
		ban->setProperty ("ActionIcon", "dialog-cancel");
		ban->setShortcut (QString ("Ctrl+B"));
		connect (ban,
				 SIGNAL (triggered ()),
				 this,
		   SLOT (handleBanTrack ()));
		TabToolbar_->addAction (ban);

		TabToolbar_->addSeparator ();

		const auto seekSlider = new SeekSlider (Player_->GetSourceObject ());
		TabToolbar_->addWidget (seekSlider);
		TabToolbar_->addSeparator ();

		const auto volumeSlider = new VolumeSlider (Player_->GetAudioOutput ());
		volumeSlider->setMinimumWidth (100);
		volumeSlider->setMaximumWidth (160);
		TabToolbar_->addWidget (volumeSlider);

		const auto effectsMenuButton = new QToolButton;
		effectsMenuButton->setMenu (EffectsMenu_);
		effectsMenuButton->setPopupMode (QToolButton::InstantPopup);
		effectsMenuButton->setProperty ("ActionIcon", "preferences-plugin");
		TabToolbar_->addWidget (effectsMenuButton);

		// fill tray menu
		connect (TrayIcon_,
				SIGNAL (changedVolume (qreal)),
				this,
				SLOT (handleChangedVolume (qreal)));
		connect (TrayIcon_,
				SIGNAL (activated (QSystemTrayIcon::ActivationReason)),
				this,
				SLOT (handleTrayIconActivated (QSystemTrayIcon::ActivationReason)));

		const auto closeLMP = new QAction (tr ("Close LMP"), TrayIcon_);
		closeLMP->setProperty ("ActionIcon", "edit-delete");
		connect (closeLMP,
				SIGNAL (triggered ()),
				this,
				SLOT (closeLMP ()));

		const auto stopAfterCurrent = new QAction (tr ("Stop after current track"), TrayIcon_);
		stopAfterCurrent->setCheckable (true);
		connect (stopAfterCurrent,
				SIGNAL (triggered ()),
				Player_,
				SLOT (stopAfterCurrent ()));

		new Util::SlotClosure<Util::NoDeletePolicy>
		{
			[this, stopAfterCurrent]
			{
				const auto& stopSource = Player_->GetCurrentStopSource ();
				const auto& current = Player_->GetSourceObject ()->GetCurrentSource ();
				stopAfterCurrent->setChecked (stopSource == current);
			},
			Player_,
			{ SIGNAL (currentStopSourceChanged ()), SIGNAL (songChanged (MediaInfo)) },
			Player_
		};

		TrayMenu_->addAction (previous);
		TrayMenu_->addAction (PlayPause_);
		TrayMenu_->addAction (stop);
		TrayMenu_->addAction (stopAfterCurrent);
		TrayMenu_->addAction (next);
		TrayMenu_->addSeparator ();
		TrayMenu_->addAction (love);
		TrayMenu_->addAction (ban);
		TrayMenu_->addSeparator ();
		TrayMenu_->addAction (closeLMP);
		TrayIcon_->setContextMenu (TrayMenu_);
	}

	void PlayerTab::Scrobble (const MediaInfo& info)
	{
		if (!XmlSettingsManager::Instance ()
				.property ("EnableScrobbling").toBool ())
			return;

		auto scrobblers = GetProxyHolder ()->GetPluginsManager ()->GetAllCastableTo<Media::IAudioScrobbler*> ();
		if (info.Title_.isEmpty () && info.Artist_.isEmpty ())
		{
			for (const auto& s : scrobblers)
				s->PlaybackStopped ();
			return;
		}

		const Media::AudioInfo aInfo = info;
		for (const auto& s : scrobblers)
			s->NowPlaying (aInfo);
	}

	void PlayerTab::FillSimilar (const Media::SimilarityInfos_t& infos)
	{
		Ui_.NPWidget_->SetSimilarArtists (infos);
	}

	void PlayerTab::RequestLyrics (const MediaInfo& info)
	{
		Ui_.NPWidget_->SetLyrics ({});

		if (!XmlSettingsManager::Instance ().property ("RequestLyrics").toBool ())
			return;

		auto finders = GetProxyHolder ()->GetPluginsManager ()->GetAllCastableTo<Media::ILyricsFinder*> ();
		if (finders.isEmpty ())
			return;

		auto opt = [] (int val) -> std::optional<int>
		{
			if (val)
				return val;
			else
				return {};
		};
		const Media::LyricsQuery query
		{
			info.Artist_,
			info.Album_,
			info.Title_,
			opt (info.Year_),
			opt (info.TrackNumber_)
		};
		for (auto finder : finders)
			Util::Sequence (this, finder->RequestLyrics (query))
					.MultipleResults (Util::Visitor
							{
								[] (const QString&) {},
								[this] (const Media::LyricsResults& results)
								{
									for (const auto& item : results)
										Ui_.NPWidget_->SetLyrics (item);
								}
							});
	}

	void PlayerTab::updateEffectsList (const QStringList& effectsList)
	{
		EffectsMenu_->clear ();

		for (int i = 0; i < effectsList.size (); ++i)
			EffectsMenu_->addAction (effectsList.at (i), [this, i] { emit effectsConfigRequested (i); });

		if (!effectsList.isEmpty ())
			EffectsMenu_->addSeparator ();

		EffectsMenu_->addAction (tr ("Open effects configuration page..."),
				[] { XmlSettingsManager::Instance ().ShowSettingsPage ("EffectsView"); });
	}

	namespace
	{
		QIcon GetIconFromState (SourceState state)
		{
			const auto mgr = GetProxyHolder ()->GetIconThemeManager ();
			switch (state)
			{
			case SourceState::Playing:
				return mgr->GetIcon ("media-playback-start");
			case SourceState::Paused:
				return mgr->GetIcon ("media-playback-pause");
			default:
				return QIcon ();
			}
		}

		template<typename T, typename F>
		void UpdateIcon (T iconable, SourceState state, F iconSizeGetter)
		{
			const QSize& iconSize = iconSizeGetter (iconable);
			if (iconSize.isEmpty ())
				return;

			QIcon icon = GetIconFromState (state);
			QIcon baseIcon = icon.isNull () ?
				QIcon ("lcicons:/lmp/resources/images/lmp.svg") :
				iconable->icon ();

			QPixmap px = baseIcon.pixmap (iconSize);
			if (px.isNull ())
				px = QPixmap (iconSize);

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
		Scrobble (info);
		RequestLyrics (info);

		if (info.Artist_.isEmpty ())
		{
			LastArtist_.clear ();
			FillSimilar (Media::SimilarityInfos_t ());
		}
		else if (!Similars_.contains (info.Artist_))
		{
			const auto& similars = GetProxyHolder ()->
					GetPluginsManager ()->GetAllCastableTo<Media::ISimilarArtists*> ();
			for (const auto similar : similars)
				Util::Sequence (this, similar->GetSimilarArtists (info.Artist_, 15)) >>
						Util::Visitor
						{
							[] (const QString& msg) { qWarning () << Q_FUNC_INFO << msg; },
							[this, artist = LastArtist_] (const Media::SimilarityInfos_t& similar)
							{
								Similars_ [artist] = similar;
								if (artist == LastArtist_)
									FillSimilar (similar);
							}
						};
		}
		else if (info.Artist_ != LastArtist_)
		{
			LastArtist_ = info.Artist_;
			FillSimilar (Similars_ [info.Artist_]);
		}
	}

	namespace
	{
		template<typename F>
		void AddToLovedBanned (const QString& trackPath,
				LocalCollection::StaticRating rating,
				F marker)
		{
			const int trackId = Core::Instance ().GetLocalCollection ()->FindTrack (trackPath);
			if (trackId >= 0)
				Core::Instance ().GetLocalCollection ()->AddTrackTo (trackId, rating);

			if (!XmlSettingsManager::Instance ()
					.property ("EnableScrobbling").toBool ())
				return;

			auto scrobblers = GetProxyHolder ()->
						GetPluginsManager ()->GetAllCastableTo<Media::IAudioScrobbler*> ();
			for (const auto scrobbler : scrobblers)
				std::invoke (marker, scrobbler);
		}
	}

	void PlayerTab::handleLoveTrack ()
	{
		AddToLovedBanned (Player_->GetCurrentMediaInfo ().LocalPath_,
				LocalCollection::StaticRating::Loved,
				&Media::IAudioScrobbler::LoveCurrentTrack);
	}

	void PlayerTab::handleBanTrack ()
	{
		AddToLovedBanned (Player_->GetCurrentMediaInfo ().LocalPath_,
				LocalCollection::StaticRating::Banned,
				&Media::IAudioScrobbler::BanCurrentTrack);
	}

	void PlayerTab::handlePlayerAvailable (bool available)
	{
		TabToolbar_->setEnabled (available);
		Ui_.Playlist_->setEnabled (available);
		Ui_.PlaylistsTab_->setEnabled (available);
		Ui_.CollectionTab_->setEnabled (available);
		Ui_.RadioTab_->setEnabled (available);
	}

	void PlayerTab::closeLMP ()
	{
		Remove ();
	}

	void PlayerTab::handleStateChanged ()
	{
		const auto newState = Player_->GetSourceObject ()->GetState ();
		if (newState == SourceState::Playing)
			PlayPause_->setProperty ("ActionIcon", "media-playback-pause");
		else
		{
			if (newState == SourceState::Stopped)
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
		qreal volume = Player_->GetAudioOutput ()->GetVolume ();
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
