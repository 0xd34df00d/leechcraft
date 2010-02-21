/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#ifndef PLUGINS_TORRENT_TABWIDGET_H
#define PLUGINS_TORRENT_TABWIDGET_H
#include <memory>
#include <QTabWidget>
#include <QAction>
#include <plugininterface/tagscompleter.h>
#include "ui_tabwidget.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace BitTorrent
		{
			class TabWidget : public QTabWidget
			{
				Q_OBJECT

				Ui::TabWidget Ui_;
				bool TorrentSelectionChanged_;
				std::auto_ptr<LeechCraft::Util::TagsCompleter> TagsChangeCompleter_;
				QAction *AddPeer_;
				QAction *BanPeer_;
				QAction *AddWebSeed_;
				QAction *RemoveWebSeed_;
			public:
				TabWidget (QAction*, QWidget* = 0);

				void InvalidateSelection ();
				void SetOverallDownloadRateController (int);
				void SetOverallUploadRateController (int);
			public slots:
				void updateTorrentStats ();
			private:
				void UpdateDashboard ();
				void UpdateOverallStats ();
				void UpdateTorrentControl ();
				void UpdateFilesPage ();
				void UpdatePeersPage ();
				void UpdatePiecesPage ();
			private slots:
				void on_OverallDownloadRateController__valueChanged (int);
				void on_OverallUploadRateController__valueChanged (int);
				void on_DesiredRating__valueChanged (double);
				void on_TorrentDownloadRateController__valueChanged (int);
				void on_TorrentUploadRateController__valueChanged (int);
				void on_TorrentDesiredRating__valueChanged (double);
				void on_TorrentManaged__stateChanged (int);
				void on_TorrentSequentialDownload__stateChanged (int);
				void on_TorrentSuperSeeding__stateChanged (int);
				void on_DownloadingTorrents__valueChanged (int);
				void on_UploadingTorrents__valueChanged (int);
				void on_TorrentTags__editingFinished ();
				void setTabWidgetSettings ();
                void currentFileChanged (const QModelIndex&);
                void on_FilePriorityRegulator__valueChanged (int);
				void handleAddPeer ();
				void handleBanPeer ();
				void handleAddWebSeed ();
				void currentPeerChanged (const QModelIndex&);
				void currentWebSeedChanged (const QModelIndex&);
				void handleRemoveWebSeed ();
			};
		};
	};
};

#endif

