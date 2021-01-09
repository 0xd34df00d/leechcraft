/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "playlistwidget.h"
#include <algorithm>
#include <QToolBar>
#include <QInputDialog>
#include <QFileDialog>
#include <QActionGroup>
#include <QToolButton>
#include <QMenu>
#include <QUndoStack>
#include <QMessageBox>
#include <QClipboard>
#include <QApplication>
#include <QKeyEvent>
#include <QSortFilterProxyModel>
#include <QTimer>
#include <QPainter>
#include <QScrollBar>
#include <util/util.h>
#include <util/xpc/util.h>
#include <util/xpc/defaulthookproxy.h>
#include <util/gui/clearlineeditaddon.h>
#include <util/sll/functional.h>
#include <util/sll/slotclosure.h>
#include <util/sll/prelude.h>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/an/ianrulesstorage.h>
#include "player.h"
#include "playlistdelegate.h"
#include "xmlsettingsmanager.h"
#include "core.h"
#include "playlistmanager.h"
#include "staticplaylistmanager.h"
#include "audiopropswidget.h"
#include "playlistundocommand.h"
#include "sortingcriteriadialog.h"
#include "util.h"
#include "palettefixerfilter.h"
#include "engine/sourceobject.h"
#include "hookinterconnector.h"
#include "playlistwidgetviewexpander.h"

Q_DECLARE_METATYPE (QList<LC::Entity>)

namespace LC
{
namespace LMP
{
	namespace
	{
		class PlaylistTreeEventFilter : public QObject
		{
			Player * const Player_;
			const QTreeView * const View_;
			const QSortFilterProxyModel * const PlaylistFilter_;

			bool HadPress_ = false;
		public:
			PlaylistTreeEventFilter (Player* player,
					QTreeView *view,
					QSortFilterProxyModel *filter,
					QObject *parent = nullptr)
			: QObject { parent }
			, Player_ { player }
			, View_ { view }
			, PlaylistFilter_ { filter }
			{
			}

			bool eventFilter (QObject*, QEvent *e)
			{
				const auto type = e->type ();
				if (type != QEvent::KeyRelease &&
						type != QEvent::KeyPress)
					return false;

				auto keyEvent = static_cast<QKeyEvent*> (e);

				const auto key = keyEvent->key ();
				const bool isSuitable = key == Qt::Key_Enter ||
						key == Qt::Key_Return ||
						(key == Qt::Key_Space && keyEvent->modifiers () == Qt::NoModifier);
				if (!isSuitable)
					return false;

				if (keyEvent->isAutoRepeat () ||
						keyEvent->count () > 1)
					return false;

				if (type == QEvent::KeyPress)
				{
					HadPress_ = true;
					return false;
				}

				if (!HadPress_)
					return false;

				HadPress_ = false;

				Player_->play (PlaylistFilter_->mapToSource (View_->currentIndex ()));
				return true;
			}
		};

		class TreeFilterModel : public QSortFilterProxyModel
		{
		public:
			TreeFilterModel (QObject *parent = 0)
			: QSortFilterProxyModel (parent)
			{
				setDynamicSortFilter (true);
			}
		protected:
			bool filterAcceptsRow (int row, const QModelIndex& parent) const
			{
				const auto& str = filterRegExp ().pattern ();
				if (str.isEmpty ())
					return true;

				auto check = [&str] (const QString& string)
				{
					return string.contains (str, Qt::CaseInsensitive);
				};

				const auto& idx = sourceModel ()->index (row, 0, parent);
				const auto& info = idx.data (Player::Role::Info).value<MediaInfo> ();
				bool isInt = false;
				if (check (info.Artist_) ||
						check (info.Album_) ||
						(info.Year_ == str.toInt (&isInt) && isInt))
					return true;

				if (check (info.Title_) || check (info.LocalPath_))
					return true;

				for (int i = 0, rc = sourceModel ()->rowCount (idx); i < rc; ++i)
					if (filterAcceptsRow (i, idx))
						return true;

				return false;
			}
		};
	}

	PlaylistWidget::PlaylistWidget (QWidget *parent)
	: QWidget (parent)
	, PlaylistToolbar_ (new QToolBar ())
	, PlaylistFilter_ (new TreeFilterModel (this))
	, UndoStack_ (new QUndoStack (this))
	{
		qRegisterMetaType<QItemSelection> ("QItemSelection");

		Ui_.setupUi (this);

		Ui_.BufferProgress_->hide ();

		connect (Ui_.SearchPlaylist_,
				SIGNAL (textChanged (QString)),
				PlaylistFilter_,
				SLOT (setFilterFixedString (QString)));

		new PlaylistWidgetViewExpander { PlaylistFilter_, [this] { expandAll (); }, this };

		connect (PlaylistFilter_,
				SIGNAL (modelReset ()),
				this,
				SLOT (expandAll ()),
				Qt::QueuedConnection);
		connect (PlaylistFilter_,
				SIGNAL (modelReset ()),
				this,
				SLOT (checkSelections ()),
				Qt::QueuedConnection);
		connect (PlaylistFilter_,
				SIGNAL (modelAboutToBeReset ()),
				this,
				SLOT (savePlayScrollPosition ()));

		Core::Instance ().GetHookInterconnector ()->RegisterHookable (this);
	}

	void PlaylistWidget::SetPlayer (Player *player, const ICoreProxy_ptr& proxy)
	{
		new Util::ClearLineEditAddon (proxy, Ui_.SearchPlaylist_);
		Ui_.Playlist_->setItemDelegate (new PlaylistDelegate (Ui_.Playlist_, Ui_.Playlist_, proxy));

		Proxy_ = proxy;
		Player_ = player;

		connect (Player_,
				SIGNAL (bufferStatusChanged (int)),
				this,
				SLOT (handleBufferStatus (int)));
		connect (Player_,
				SIGNAL (songChanged (MediaInfo)),
				this,
				SLOT (handleSongChanged (MediaInfo)));

		const auto model = Player_->GetPlaylistModel ();
		PlaylistFilter_->setSourceModel (model);

		Ui_.Playlist_->setModel (PlaylistFilter_);
		Ui_.Playlist_->expandAll ();

		connect (Ui_.Playlist_,
				SIGNAL (doubleClicked (QModelIndex)),
				this,
				SLOT (play (QModelIndex)));
		connect (Player_,
				SIGNAL (insertedAlbum (QModelIndex)),
				this,
				SLOT (expand (QModelIndex)));

		Ui_.PlaylistLayout_->addWidget (PlaylistToolbar_);

		InitCommonActions ();
		InitViewActions ();
		InitToolbarActions ();

		connect (model,
				SIGNAL (rowsInserted (QModelIndex, int, int)),
				this,
				SLOT (updateStatsLabel ()),
				Qt::QueuedConnection);
		connect (model,
				SIGNAL (rowsRemoved (QModelIndex, int, int)),
				this,
				SLOT (updateStatsLabel ()),
				Qt::QueuedConnection);
		connect (model,
				SIGNAL (modelReset ()),
				this,
				SLOT (updateStatsLabel ()),
				Qt::QueuedConnection);

		connect (Ui_.Playlist_->selectionModel (),
				SIGNAL (selectionChanged (QItemSelection, QItemSelection)),
				this,
				SLOT (updateStatsLabel ()),
				Qt::QueuedConnection);
		updateStatsLabel ();

		connect (Ui_.Playlist_->selectionModel (),
				SIGNAL (currentChanged (QModelIndex, QModelIndex)),
				this,
				SLOT (updateDownloadAction ()));
		connect (Ui_.Playlist_->selectionModel (),
				SIGNAL (selectionChanged (QItemSelection, QItemSelection)),
				this,
				SLOT (updateDownloadAction ()));

		Ui_.Playlist_->installEventFilter (new PlaylistTreeEventFilter (Player_,
					Ui_.Playlist_,
					PlaylistFilter_));

		new PaletteFixerFilter (Ui_.Playlist_);

		connect (player,
				SIGNAL (shouldClearFiltering ()),
				Ui_.SearchPlaylist_,
				SLOT (clear ()));
	}

	void PlaylistWidget::InitCommonActions ()
	{
		ActionDownloadTrack_ = new QAction (tr ("Download..."), this);
		ActionDownloadTrack_->setProperty ("ActionIcon", "download");
		connect (ActionDownloadTrack_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleDownload ()));
	}

	void PlaylistWidget::InitToolbarActions ()
	{
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

		QAction *addURL = new QAction (tr ("Add URL..."), this);
		addURL->setProperty ("ActionIcon", "folder-remote");
		connect (addURL,
				SIGNAL (triggered ()),
				this,
				SLOT (addURL ()));
		PlaylistToolbar_->addAction (addURL);

		PlaylistToolbar_->addSeparator ();
		PlaylistToolbar_->addAction (ActionDownloadTrack_);
		PlaylistToolbar_->addSeparator ();

		ActionMoveTop_ = new QAction (tr ("Move tracks to top"), Ui_.Playlist_);
		ActionMoveTop_->setProperty ("ActionIcon", "go-top");
		connect (ActionMoveTop_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleMoveTop ()));

		ActionMoveUp_ = new QAction (tr ("Move tracks up"), Ui_.Playlist_);
		ActionMoveUp_->setProperty ("ActionIcon", "go-up");
		ActionMoveUp_->setShortcut (QString ("Ctrl+Up"));
		connect (ActionMoveUp_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleMoveUp ()));

		ActionMoveDown_ = new QAction (tr ("Move tracks down"), Ui_.Playlist_);
		ActionMoveDown_->setProperty ("ActionIcon", "go-down");
		ActionMoveDown_->setShortcut (QString ("Ctrl+Down"));
		connect (ActionMoveDown_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleMoveDown ()));

		ActionMoveBottom_ = new QAction (tr ("Move tracks to bottom"), Ui_.Playlist_);
		ActionMoveBottom_->setProperty ("ActionIcon", "go-bottom");
		connect (ActionMoveBottom_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleMoveBottom ()));

		auto moveUpButton = new QToolButton;
		moveUpButton->setDefaultAction (ActionMoveUp_);
		moveUpButton->setMenu (new QMenu);
		moveUpButton->menu ()->addAction (ActionMoveTop_);

		auto moveDownButton = new QToolButton;
		moveDownButton->setDefaultAction (ActionMoveDown_);
		moveDownButton->setMenu (new QMenu);
		moveDownButton->menu ()->addAction (ActionMoveBottom_);

		SetPlayModeButton ();
		SetSortOrderButton ();

		auto shuffleAction = new QAction (tr ("Shuffle tracks"), Ui_.Playlist_);
		shuffleAction->setProperty ("ActionIcon", "media-playlist-shuffle");
		connect (shuffleAction,
				SIGNAL (triggered ()),
				Player_,
				SLOT (shufflePlaylist ()));
		PlaylistToolbar_->addAction (shuffleAction);

		MoveUpButtonAction_ = PlaylistToolbar_->addWidget (moveUpButton);
		MoveDownButtonAction_ = PlaylistToolbar_->addWidget (moveDownButton);
		EnableMoveButtons (false);

		PlaylistToolbar_->addSeparator ();

		auto undo = UndoStack_->createUndoAction (this);
		undo->setProperty ("ActionIcon", "edit-undo");
		undo->setShortcut (QKeySequence ("Ctrl+Z"));
		PlaylistToolbar_->addAction (undo);
		auto redo = UndoStack_->createRedoAction (this);
		redo->setProperty ("ActionIcon", "edit-redo");
		PlaylistToolbar_->addAction (redo);

		PlaylistToolbar_->addSeparator ();

		PlaylistToolbar_->addAction (ActionToggleSearch_);
	}

	void PlaylistWidget::SetPlayModeButton ()
	{
		auto playButton = new QToolButton;
		playButton->setIcon (Proxy_->GetIconThemeManager ()->GetIcon ("view-media-playlist"));
		playButton->setPopupMode (QToolButton::InstantPopup);
		QMenu *playMode = new QMenu (tr ("Play mode"));
		playButton->setMenu (playMode);

		const std::vector<std::pair<Player::PlayMode, QString>> modes =
		{
			{ Player::PlayMode::Sequential, tr ("Sequential") },
			{ Player::PlayMode::Shuffle, tr ("Shuffle") },
			{ Player::PlayMode::ShuffleAlbums, tr ("Shuffle albums") },
			{ Player::PlayMode::ShuffleArtists, tr ("Shuffle artists") },
			{ Player::PlayMode::RepeatTrack, tr ("Repeat track") },
			{ Player::PlayMode::RepeatAlbum, tr ("Repeat album") },
			{ Player::PlayMode::RepeatWhole, tr ("Repeat whole") }
		};
		PlayModesGroup_ = new QActionGroup (this);
		bool hadChecked = false;
		for (const auto& pair : modes)
		{
			QAction *action = new QAction (pair.second, this);
			action->setProperty ("PlayMode", static_cast<int> (pair.first));
			action->setCheckable (true);
			action->setActionGroup (PlayModesGroup_);

			if (!hadChecked)
			{
				action->setChecked (true);
				hadChecked = true;
			}

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
	}

	void PlaylistWidget::SetSortOrderButton ()
	{
		auto sortButton = new QToolButton;
		sortButton->setIcon (Proxy_->GetIconThemeManager ()->GetIcon ("view-sort-ascending"));
		sortButton->setPopupMode (QToolButton::InstantPopup);

		auto menu = new QMenu (tr ("Sorting"));
		sortButton->setMenu (menu);

		auto getInts = [] (const QList<SortingCriteria>& crit)
		{
			return Util::Map (crit, [] (auto item) { return QVariant { static_cast<int> (item) }; });
		};

		const auto stdSorts =
		{
			QPair<QString, QList<SortingCriteria>>
			{
				tr ("Artist / Year / Album / Track number"),
				{
					SortingCriteria::Artist,
					SortingCriteria::Year,
					SortingCriteria::Album,
					SortingCriteria::TrackNumber
				}
			},
			{
				tr ("Artist / Track title"),
				{
					SortingCriteria::Artist,
					SortingCriteria::TrackTitle
				}
			},
			{
				tr ("File path"),
				{
					SortingCriteria::DirectoryPath,
					SortingCriteria::FileName
				}
			},
			{
				tr ("No sort"),
				{}
			}
		};

		const auto& currentCriteria = Player_->GetSortingCriteria ();

		auto sortGroup = new QActionGroup (this);
		bool wasChecked = false;
		for (const auto& pair : stdSorts)
		{
			auto act = menu->addAction (pair.first);
			act->setProperty ("SortInts", getInts (pair.second));
			act->setCheckable (true);
			sortGroup->addAction (act);
			if (pair.second == currentCriteria)
			{
				act->setChecked (true);
				wasChecked = true;
			}
			else
				act->setChecked (false);

			connect (act,
					SIGNAL (triggered ()),
					this,
					SLOT (handleStdSort ()));
		}

		menu->addSeparator ();
		auto customAct = menu->addAction (tr ("Custom..."));
		customAct->setCheckable (true);
		if (!wasChecked)
			customAct->setChecked (true);
		sortGroup->addAction (customAct);
		connect (customAct,
				SIGNAL (triggered ()),
				this,
				SLOT (handleCustomSort ()));

		PlaylistToolbar_->addWidget (sortButton);
	}

	void PlaylistWidget::InitViewActions ()
	{
		ActionRemoveSelected_ = new QAction (tr ("Delete from playlist"), Ui_.Playlist_);
		ActionRemoveSelected_->setProperty ("ActionIcon", "list-remove");
		ActionRemoveSelected_->setShortcut (Qt::Key_Delete);
		ActionRemoveSelected_->setShortcutContext (Qt::WidgetShortcut);
		connect (ActionRemoveSelected_,
				SIGNAL (triggered ()),
				this,
				SLOT (removeSelectedSongs ()));
		Ui_.Playlist_->addAction (ActionRemoveSelected_);

		ActionStopAfterSelected_ = new QAction (tr ("Stop after this track"), Ui_.Playlist_);
		ActionStopAfterSelected_->setProperty ("ActionIcon", "media-playback-stop");
		connect (ActionStopAfterSelected_,
				SIGNAL (triggered ()),
				this,
				SLOT (setStopAfterSelected ()));

		ActionAddToOneShot_ = new QAction (tr ("Add to instant queue"), Ui_.Playlist_);
		ActionAddToOneShot_->setProperty ("ActionIcon", "list-add");
		connect (ActionAddToOneShot_,
				SIGNAL (triggered ()),
				this,
				SLOT (addToOneShot ()));

		ActionRemoveFromOneShot_ = new QAction (tr ("Remove from instant queue"), Ui_.Playlist_);
		ActionRemoveFromOneShot_->setProperty ("ActionIcon", "list-remove");
		connect (ActionRemoveFromOneShot_,
				SIGNAL (triggered ()),
				this,
				SLOT (removeFromOneShot ()));

		ActionMoveOneShotUp_ = new QAction (tr ("Move up in instant queue"), Ui_.Playlist_);
		ActionMoveOneShotUp_->setProperty ("ActionIcon", "go-up");
		connect (ActionMoveOneShotUp_,
				SIGNAL (triggered ()),
				this,
				SLOT (moveOneShotUp ()));

		ActionMoveOneShotDown_ = new QAction (tr ("Move down in instant queue"), Ui_.Playlist_);
		ActionMoveOneShotDown_->setProperty ("ActionIcon", "go-down");
		connect (ActionMoveOneShotDown_,
				SIGNAL (triggered ()),
				this,
				SLOT (moveOneShotDown ()));

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

		TrackActions_ = new QMenu (tr ("Track actions"));
		TrackActions_->addAction (tr ("Perform action after this track starts..."),
				this, SLOT (initPerformAfterTrackStart ()));
		TrackActions_->addAction (tr ("Perform action after this track stops..."),
				this, SLOT (initPerformAfterTrackStop ()));

		ExistingTrackActions_ = TrackActions_->addMenu (tr ("Existing"));
		connect (ExistingTrackActions_,
				SIGNAL (triggered (QAction*)),
				this,
				SLOT (handleExistingTrackAction (QAction*)));

		ActionToggleSearch_ = new QAction (tr ("Toggle search field"), Ui_.Playlist_);
		ActionToggleSearch_->setShortcut (QKeySequence::Find);
		ActionToggleSearch_->setCheckable (true);
		ActionToggleSearch_->setProperty ("ActionIcon", "edit-find");
		connect (ActionToggleSearch_,
				SIGNAL (toggled (bool)),
				Ui_.SearchPlaylist_,
				SLOT (setVisible (bool)));
		connect (ActionToggleSearch_,
				SIGNAL (toggled (bool)),
				Ui_.SearchPlaylist_,
				SLOT (setFocus ()));
		connect (ActionToggleSearch_,
				SIGNAL (toggled (bool)),
				Ui_.SearchPlaylist_,
				SLOT (clear ()));
		Ui_.SearchPlaylist_->setVisible (false);
	}

	void PlaylistWidget::EnableMoveButtons (bool enabled)
	{
		MoveUpButtonAction_->setEnabled (enabled);
		MoveDownButtonAction_->setEnabled (enabled);
	}

	QList<AudioSource> PlaylistWidget::GetSelected () const
	{
		auto selected = Ui_.Playlist_->selectionModel ()->selectedRows ();
		if (selected.isEmpty ())
			selected << Ui_.Playlist_->currentIndex ();

		QList<AudioSource> sources;
		for (const auto& index : selected)
			sources += Player_->GetIndexSources (PlaylistFilter_->mapToSource (index));
		return sources;
	}

	void PlaylistWidget::SelectSources (const QList<AudioSource>& sources)
	{
		auto tryIdx = [&sources, this] (const QModelIndex& idx)
		{
			if (sources.contains (Player_->GetIndexSources (idx).value (0)))
				Ui_.Playlist_->selectionModel ()->select (PlaylistFilter_->mapFromSource (idx),
						QItemSelectionModel::Select | QItemSelectionModel::Rows);
		};

		auto plModel = Player_->GetPlaylistModel ();
		for (int i = 0; i < plModel->rowCount (); ++i)
		{
			const auto& albumIdx = plModel->index (i, 0);

			const int tracks = plModel->rowCount (albumIdx);
			if (!tracks)
				tryIdx (albumIdx);
			else
				for (int j = 0; j < tracks; ++j)
					tryIdx (plModel->index (j, 0, albumIdx));
		}
	}

	void PlaylistWidget::focusIndex (const QModelIndex& index)
	{
		if (!XmlSettingsManager::Instance ().property ("AutocenterCurrentTrack").toBool ())
			return;

		Ui_.Playlist_->scrollTo (PlaylistFilter_->mapFromSource (index),
				QAbstractItemView::PositionAtCenter);
	}

	namespace
	{
		QIcon SymbolToIcon (const QPair<QString, QColor>& symb, const QFontMetrics& fm)
		{
			const auto& rect = fm.boundingRect (symb.first);

			QPixmap px { rect.size () };
			px.fill (Qt::transparent);
			{
				QPainter painter { &px };
				if (symb.second.isValid ())
					painter.setPen (symb.second);
				painter.drawText (QRect { { 0, 0 }, rect.size () },
						Qt::AlignCenter | Qt::AlignHCenter,
						symb.first);
			}

			QIcon icon;
			icon.addPixmap (px);
			return icon;
		}
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
			menu->addAction (ActionShowTrackProps_);

		menu->addSeparator ();
		menu->addAction (ActionStopAfterSelected_);

		const auto& oneShotPosVar = idx.data (Player::Role::OneShotPos);
		if (!oneShotPosVar.isValid ())
			menu->addAction (ActionAddToOneShot_);
		else
		{
			menu->addAction (ActionRemoveFromOneShot_);

			if (oneShotPosVar.toInt () > 0)
				menu->addAction (ActionMoveOneShotUp_);
			if (oneShotPosVar.toInt () < Player_->GetOneShotQueueSize () - 1)
				menu->addAction (ActionMoveOneShotDown_);
		}

		menu->addMenu (TrackActions_);
		const auto& existingRules = idx.data (Player::Role::MatchingRules).value<QList<Entity>> ();
		ExistingTrackActions_->menuAction ()->setVisible (!existingRules.isEmpty ());

		ExistingTrackActions_->clear ();
		for (const auto& rule : existingRules)
		{
			const auto action = ExistingTrackActions_->addAction (rule.Entity_.toString ());
			action->setProperty ("LMP/SourceRule", QVariant::fromValue (rule));

			const auto& symbol = GetRuleSymbol (rule);
			action->setIcon (SymbolToIcon (symbol, menu->fontMetrics ()));
		}

		menu->addSeparator ();

		if (updateDownloadAction ())
		{
			menu->addAction (ActionDownloadTrack_);
			menu->addSeparator ();
		}

		menu->addAction (ActionToggleSearch_);

		auto mediaInfo = idx.data (Player::Role::Info).value<MediaInfo> ();
		if (idx.model ()->rowCount (idx))
			mediaInfo = idx.model ()->index (0, 0, idx).data (Player::Role::Info).value<MediaInfo> ();

		emit hookPlaylistContextMenuRequested (std::make_shared<Util::DefaultHookProxy> (),
				menu,
				mediaInfo);

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
		for (auto action : PlayModesGroup_->actions ())
			if (action->property ("PlayMode").toInt () == static_cast<int> (mode))
			{
				action->setChecked (true);
				return;
			}
	}

	void PlaylistWidget::play (const QModelIndex& index)
	{
		Player_->play (PlaylistFilter_->mapToSource (index));
	}

	void PlaylistWidget::expand (const QModelIndex& index)
	{
		Ui_.Playlist_->expand (PlaylistFilter_->mapFromSource (index));
	}

	void PlaylistWidget::expandAll ()
	{
		Ui_.Playlist_->expandAll ();
		checkSelections ();
	}

	void PlaylistWidget::checkSelections ()
	{
		if (NextResetSelect_.isEmpty () || !PlaylistFilter_->rowCount ())
			return;

		SelectSources (NextResetSelect_);
		NextResetSelect_.clear ();
	}

	void PlaylistWidget::handleBufferStatus (int status)
	{
		Ui_.BufferProgress_->setValue (status);
		Ui_.BufferProgress_->setVisible (status > 0 && status < 100);
	}

	void PlaylistWidget::handleSongChanged (const MediaInfo& info)
	{
		if (!info.LocalPath_.isEmpty ())
			handleBufferStatus (100);
	}

	void PlaylistWidget::handleStdSort ()
	{
		const auto& intVars = sender ()->property ("SortInts").toList ();
		const auto& criteria = Util::Map (intVars,
				[] (const QVariant& var) { return static_cast<SortingCriteria> (var.toInt ()); });
		Player_->SetSortingCriteria (criteria);

		EnableMoveButtons (criteria.isEmpty ());
	}

	void PlaylistWidget::handleCustomSort ()
	{
		const auto& var = XmlSettingsManager::Instance ().property ("LastCustomSortCriteria");
		auto lastCustom = LoadCriteria (var);

		const auto& current = Player_->GetSortingCriteria ();
		if (lastCustom.isEmpty ())
			lastCustom = current;

		SortingCriteriaDialog dia (this);
		dia.SetCriteria (lastCustom);
		if (dia.exec () != QDialog::Accepted)
			return;

		const auto& newCriteria = dia.GetCriteria ();
		if (!newCriteria.isEmpty ())
		{
			const auto& var = SaveCriteria (newCriteria);
			XmlSettingsManager::Instance ().setProperty ("LastCustomSortCriteria", var);
		}

		if (newCriteria != current)
			Player_->SetSortingCriteria (newCriteria);
	}

	void PlaylistWidget::savePlayScrollPosition ()
	{
		const auto bar = Ui_.Playlist_->verticalScrollBar ();
		if (!bar)
			return;

		const auto val = bar->value ();

		new Util::SlotClosure<Util::DeleteLaterPolicy>
		{
			[bar, val]
			{
				QTimer::singleShot (0, bar, [=] { bar->setValue (std::min (val, bar->maximum ())); });
			},
			PlaylistFilter_,
			SIGNAL (modelReset ()),
			this
		};
	}

	void PlaylistWidget::removeSelectedSongs ()
	{
		const auto& removedSources = GetSelected ();
		const auto& title = tr ("Remove %n song(s)", 0, removedSources.size ());

		auto cmd = new PlaylistUndoCommand (title, removedSources, Player_);
		UndoStack_->push (cmd);
	}

	void PlaylistWidget::setStopAfterSelected ()
	{
		const auto& index = PlaylistFilter_->mapToSource (Ui_.Playlist_->currentIndex ());
		if (!index.isValid ())
			return;

		Player_->SetStopAfter (index);
	}

	void PlaylistWidget::addToOneShot ()
	{
		auto selected = Ui_.Playlist_->selectionModel ()->selectedRows (0);
		if (selected.isEmpty ())
			selected << Ui_.Playlist_->currentIndex ();
		const auto& mapped = Util::Map (selected,
				[this] (const QModelIndex& index) { return PlaylistFilter_->mapToSource (index); });
		if (mapped.isEmpty ())
			return;

		for (const auto& index : mapped)
			Player_->AddToOneShotQueue (index);
	}

	void PlaylistWidget::removeFromOneShot ()
	{
		auto selection = Ui_.Playlist_->selectionModel ()->selectedRows ();
		const auto& current = Ui_.Playlist_->currentIndex ();
		if (!selection.contains (current) && current.isValid ())
			selection << current;

		for (const auto& index : selection)
			Player_->RemoveFromOneShotQueue (PlaylistFilter_->mapToSource (index));
	}

	void PlaylistWidget::moveOneShotUp ()
	{
		const auto& index = PlaylistFilter_->mapToSource (Ui_.Playlist_->currentIndex ());
		if (!index.isValid ())
			return;

		Player_->OneShotMoveUp (index);
	}

	void PlaylistWidget::moveOneShotDown ()
	{
		const auto& index = PlaylistFilter_->mapToSource (Ui_.Playlist_->currentIndex ());
		if (!index.isValid ())
			return;

		Player_->OneShotMoveDown (index);
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

	namespace
	{
		void EmitStateRule (const QModelIndex& index, const QString& state,
				const QString& nameTempl, const ICoreProxy_ptr& proxy)
		{
			const auto& info = index.data (Player::Role::Info).value<MediaInfo> ();

			auto url = info.Additional_.value ("URL").toUrl ();
			if (url.isEmpty ())
				url = QUrl::fromLocalFile (info.LocalPath_);

			const auto& e = Util::MakeANRule (nameTempl
						.arg (info.Title_)
						.arg (info.Artist_),
					"org.LeechCraft.LMP",
					AN::CatMediaPlayer,
					{ AN::TypeMediaPlaybackStatus },
					AN::NotifySingleShot,
					true,
					{
						{
							AN::Field::MediaPlaybackStatus,
							ANStringFieldValue { state }
						},
						{
							AN::Field::MediaArtist,
							ANStringFieldValue { info.Artist_ }
						},
						{
							AN::Field::MediaAlbum,
							ANStringFieldValue { info.Album_ }
						},
						{
							AN::Field::MediaTitle,
							ANStringFieldValue { info.Title_ }
						},
						{
							AN::Field::MediaLength,
							ANIntFieldValue { info.Length_, ANIntFieldValue::OEqual }
						},
						{
							AN::Field::MediaPlayerURL,
							ANStringFieldValue { url.toEncoded () }
						}
					});
			proxy->GetEntityManager ()->HandleEntity (e);
		}
	}

	void PlaylistWidget::initPerformAfterTrackStart ()
	{
		EmitStateRule (Ui_.Playlist_->currentIndex (),
				"Playing",
				tr ("Perform when %1 by %2 starts playing"),
				Proxy_);
	}

	void PlaylistWidget::initPerformAfterTrackStop ()
	{
		EmitStateRule (Ui_.Playlist_->currentIndex (),
				"Stopped",
				tr ("Perform when %1 by %2 stops playing"),
				Proxy_);
	}

	void PlaylistWidget::handleExistingTrackAction (QAction *action)
	{
		const auto& rule = action->property ("LMP/SourceRule").value<Entity> ();
		const auto& pluginId = rule.Additional_ ["org.LC.AdvNotifications.SenderID"].toByteArray ();

		const auto pluginMgr = Proxy_->GetPluginsManager ();
		const auto pluginObj = pluginMgr->GetPluginByID (pluginId);
		if (!pluginObj)
		{
			qWarning () << Q_FUNC_INFO
					<< "plugin"
					<< pluginId
					<< "not found";
			return;
		}

		const auto irs = qobject_cast<IANRulesStorage*> (pluginObj);
		irs->RequestRuleConfiguration (rule);
	}

	void PlaylistWidget::handleMoveUp ()
	{
		const auto& sources = GetSelected ();

		if (sources.isEmpty ())
			return;

		auto allSrcs = Player_->GetQueue ();
		for (int i = 1, size = allSrcs.size (); i < size; ++i)
			if (sources.contains (allSrcs.at (i)))
				std::swap (allSrcs [i], allSrcs [i - 1]);

		Player_->Enqueue (allSrcs, Player::EnqueueReplace);

		NextResetSelect_ = sources;
	}

	void PlaylistWidget::handleMoveTop ()
	{
		const auto& sources = GetSelected ();
		auto allSrcs = Player_->GetQueue ();
		for (const auto& source : sources)
			allSrcs.removeAll (source);

		Player_->Enqueue (sources + allSrcs, Player::EnqueueReplace);
		NextResetSelect_ = sources;
	}

	void PlaylistWidget::handleMoveDown ()
	{
		const auto& sources = GetSelected ();

		if (sources.isEmpty ())
			return;

		auto allSrcs = Player_->GetQueue ();
		for (int i = allSrcs.size () - 2; i >= 0; --i)
			if (sources.contains (allSrcs.at (i)))
				std::swap (allSrcs [i], allSrcs [i + 1]);

		Player_->Enqueue (allSrcs, Player::EnqueueReplace);

		NextResetSelect_ = sources;
	}

	void PlaylistWidget::handleMoveBottom ()
	{
		const auto& sources = GetSelected ();
		auto allSrcs = Player_->GetQueue ();
		for (const auto& source : sources)
			allSrcs.removeAll (source);

		Player_->Enqueue (allSrcs + sources, Player::EnqueueReplace);
		NextResetSelect_ = sources;
	}

	void PlaylistWidget::handleSavePlaylist ()
	{
		const auto& name = QInputDialog::getText (this,
				tr ("Save playlist"),
				tr ("Enter name for the playlist:"));
		if (name.isEmpty ())
			return;

		auto mgr = Core::Instance ().GetPlaylistManager ()->GetStaticManager ();

		if (mgr->EnumerateCustomPlaylists ().contains (name) &&
				QMessageBox::question (this,
						"LeechCraft",
						tr ("Playlist %1 already exists. Do you want to overwrite it?")
							.arg ("<em>" + name + "</em>"),
						QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
			return;

		mgr->SaveCustomPlaylist (name, Player_->GetAsNativePlaylist ());
	}

	void PlaylistWidget::loadFromDisk ()
	{
		auto prevPath = XmlSettingsManager::Instance ()
				.Property ("PrevAddToPlaylistPath", QDir::homePath ()).toString ();
		auto files = QFileDialog::getOpenFileNames (this,
				tr ("Load files"),
				prevPath,
				QString { "%1 (*.ogg *.flac *.mp3 *.wav);;%2 (*.pls *.m3u *.m3u8 *.xspf);;%3 (*.*)" }
					.arg (tr ("Music files"))
					.arg (tr ("Playlists"))
					.arg (tr ("All files")));
		if (files.isEmpty ())
			return;

		prevPath = QFileInfo (files.at (0)).absoluteDir ().absolutePath ();
		XmlSettingsManager::Instance ().setProperty ("PrevAddToPlaylistPath", prevPath);

		Player_->Enqueue (files);
	}

	void PlaylistWidget::addURL ()
	{
		auto cb = qApp->clipboard ();
		QString textCb = cb->text (QClipboard::Selection);
		if (textCb.isEmpty () || !QUrl (textCb).isValid ())
			textCb = cb->text (QClipboard::Selection);
		if (!QUrl (textCb).isValid ())
			textCb.clear ();

		const auto& url = QInputDialog::getText (this,
				"LeechCraft",
				tr ("Enter URL to add to the play queue:"),
				QLineEdit::Normal,
				textCb);
		if (url.isEmpty ())
			return;

		QUrl urlObj (url);
		if (!urlObj.isValid ())
		{
			QMessageBox::warning (this,
					"LeechCraft",
					tr ("Invalid URL."));
			return;
		}

		Player_->Enqueue ({ urlObj });
	}

	namespace
	{
		QList<AudioSource> GetSelectedOrCurrent (const QList<AudioSource>& selected, Player *player)
		{
			if (!selected.isEmpty ())
				return selected;

			return { player->GetSourceObject ()->GetCurrentSource () };
		}
	}

	bool PlaylistWidget::updateDownloadAction ()
	{
		const auto& selected = GetSelectedOrCurrent (GetSelected (), Player_);
		const bool hasRemote = std::any_of (selected.begin (), selected.end (),
				[] (const AudioSource& src) { return src.IsRemote (); });

		ActionDownloadTrack_->setEnabled (hasRemote);

		return hasRemote;
	}

	void PlaylistWidget::handleDownload ()
	{
		const auto& remotes = Util::Filter (GetSelectedOrCurrent (GetSelected (), Player_), &AudioSource::IsRemote);
		if (remotes.isEmpty ())
			return;

		GrabTracks (Util::Map (remotes, Util::BindMemFn (&Player::GetMediaInfo, Player_)), this);
	}

	void PlaylistWidget::updateStatsLabel ()
	{
		const int tracksCount = Player_->GetQueue ().size ();

		auto model = Player_->GetPlaylistModel ();
		int length = 0;
		for (int i = 0, rc = model->rowCount (); i < rc; ++i)
		{
			const auto& idx = model->index (i, 0);
			length += model->rowCount (idx) ?
					idx.data (Player::Role::AlbumLength).toInt () :
					idx.data (Player::Role::Info).value<MediaInfo> ().Length_;
		}

		QModelIndexList selectedTracks;
		for (const auto& idx : Ui_.Playlist_->selectionModel ()->selectedRows ())
			if (!model->rowCount (idx))
				selectedTracks << idx;

		int selectedLength = 0;
		if (selectedTracks.size () > 1)
			for (const auto& idx : selectedTracks)
				selectedLength += idx.data (Player::Role::Info).value<MediaInfo> ().Length_;

		QString text;
		if (selectedLength > 0)
			text = tr ("%n track(s), total duration: %1; selected duration: %2", 0, tracksCount)
					.arg (Util::MakeTimeFromLong (length))
					.arg (Util::MakeTimeFromLong (selectedLength));
		else
			text = tr ("%n track(s), total duration: %1", 0, tracksCount)
					.arg (Util::MakeTimeFromLong (length));
		Ui_.StatsLabel_->setText (text);
	}
}
}
