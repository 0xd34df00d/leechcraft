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
				QAction *AddWebSeed_;
				QAction *RemoveWebSeed_;
			public:
				TabWidget (QWidget* = 0);

				void InvalidateSelection ();
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
				void handleAddPeer ();
				void handleAddWebSeed ();
				void currentWebSeedChanged (const QModelIndex&);
				void handleRemoveWebSeed ();
			};
		};
	};
};

#endif

