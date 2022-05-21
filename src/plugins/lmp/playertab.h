/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QWidget>
#include <interfaces/ihavetabs.h>
#include <interfaces/media/audiostructs.h>
#include <interfaces/ihaverecoverabletabs.h>
#include "lmpsystemtrayicon.h"
#include "player.h"
#include "ui_playertab.h"

class QStandardItemModel;
class QListWidget;
class QTabBar;

namespace LC
{
struct Entity;

namespace LMP
{
	struct MediaInfo;
	class Player;
	class NowPlayingPixmapHandler;

	class PlayerTab : public QWidget
					, public ITabWidget
					, public IRecoverableTab
	{
		Q_OBJECT
		Q_INTERFACES (ITabWidget IRecoverableTab)

		Ui::PlayerTab Ui_;

		QObject *Plugin_;
		const TabClassInfo TC_;

		Player *Player_;

		QToolBar *TabToolbar_;

		QHash<QString, Media::SimilarityInfos_t> Similars_;
		QString LastArtist_;

		LMPSystemTrayIcon *TrayIcon_;
		QAction *PlayPause_;
		QMenu *TrayMenu_;

		QListWidget *NavButtons_;
		QTabBar *NavBar_;

		NowPlayingPixmapHandler *NPPixmapHandler_;

		QMenu * const EffectsMenu_;
	public:
		PlayerTab (const TabClassInfo&, Player*, const ICoreProxy_ptr&, QObject*, QWidget* = 0);
		~PlayerTab ();

		TabClassInfo GetTabClassInfo () const;
		QObject* ParentMultiTabs ();
		void Remove ();
		QToolBar* GetToolBar () const;

		Player* GetPlayer () const;

		QByteArray GetTabRecoverData () const;
		QIcon GetTabRecoverIcon () const;
		QString GetTabRecoverName () const;

		void AddNPTab (const QString&, QWidget*);

		void InitWithOtherPlugins ();
	private:
		void SetupNavButtons ();
		void SetupToolbar ();
		void Scrobble (const MediaInfo&);
		void FillSimilar (const Media::SimilarityInfos_t&);
		void RequestLyrics (const MediaInfo&);
	public slots:
		void updateEffectsList (const QStringList&);
	private slots:
		void handleSongChanged (const MediaInfo&);
		void handleLoveTrack ();
		void handleBanTrack ();

		void handlePlayerAvailable (bool);

		void handleStateChanged ();
		void handleShowTrayIcon ();
		void handleUseNavTabBar ();
		void handleTrayIconActivated (QSystemTrayIcon::ActivationReason reason);
	signals:
		void removeTab ();

		void fullRaiseRequested ();

		void tabRecoverDataChanged ();

		void effectsConfigRequested (int);

		// Internal signal.
		void notifyCurrentTrackRequested ();
	};
}
}
