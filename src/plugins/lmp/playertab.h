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
#include <interfaces/ihavetabs.h>
#include <interfaces/media/audiostructs.h>
#include "ui_playertab.h"

class QStandardItemModel;
class QFileSystemModel;

namespace LeechCraft
{
struct Entity;

namespace LMP
{
	class MediaInfo;
	class Player;

	class PlayerTab : public QWidget
					, public ITabWidget
	{
		Q_OBJECT
		Q_INTERFACES (ITabWidget);

		Ui::PlayerTab Ui_;

		QObject *Plugin_;
		const TabClassInfo TC_;

		QFileSystemModel *FSModel_;

		Player *Player_;
		QToolBar *PlaylistToolbar_;

		QToolBar *TabToolbar_;

		QHash<QString, Media::SimilarityInfos_t> Similars_;
		QStandardItemModel *SimilarsModel_;
	public:
		PlayerTab (const TabClassInfo&, QObject*, QWidget* = 0);

		TabClassInfo GetTabClassInfo () const;
		QObject* ParentMultiTabs ();
		void Remove ();
		QToolBar* GetToolBar () const;
	private:
		void SetupToolbar ();
		void SetupCollection ();
		void SetupFSBrowser ();
		void SetupPlaylist ();
		void FillSimilar (Media::SimilarityInfos_t);
	private slots:
		void handleSongChanged (const MediaInfo&);
		void handleSimilarError ();
		void handleSimilarReady ();
		void handleScanProgress (int);
		void handleChangePlayMode ();
		void loadFromCollection ();
		void loadFromFSBrowser ();
		void loadFromDisk ();
	signals:
		void changeTabName (QWidget*, const QString&);
		void removeTab (QWidget*);

		void gotEntity (const LeechCraft::Entity&);
	};
}
}
