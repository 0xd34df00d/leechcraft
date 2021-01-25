/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QTabWidget>
#include "ui_torrenttabwidget.h"

namespace LC
{
namespace BitTorrent
{
	class TorrentTabWidget : public QTabWidget
	{
		Q_OBJECT

		Ui::TorrentTabWidget Ui_;
		QAction *AddPeer_;
		QAction *BanPeer_;
		QAction *AddWebSeed_;
		QAction *RemoveWebSeed_;
		int Index_ = -1;
		QList<int> SelectedIndices_;

	public:
		TorrentTabWidget (QWidget* = 0);

		void SetChangeTrackersAction (QAction*);

		void SetCurrentIndex (int);
		void SetSelectedIndices (const QList<int>&);
		void InvalidateSelection ();
	public slots:
		void updateTorrentStats ();
	private:
		template<typename F>
		void ForEachSelected (F&&) const;

		void UpdateDashboard ();
		void UpdateOverallStats ();
		void UpdateTorrentControl ();
	private slots:
		void on_OverallDownloadRateController__valueChanged (int);
		void on_OverallUploadRateController__valueChanged (int);
		void on_TorrentDownloadRateController__valueChanged (int);
		void on_TorrentUploadRateController__valueChanged (int);
		void on_TorrentManaged__stateChanged (int);
		void on_TorrentSequentialDownload__stateChanged (int);
		void on_TorrentSuperSeeding__stateChanged (int);
		void on_DownloadingTorrents__valueChanged (int);
		void on_UploadingTorrents__valueChanged (int);
		void on_TorrentTags__editingFinished ();

		void setTabWidgetSettings ();

		void on_LabelComment__linkActivated (const QString&);

		void handleAddWebSeed ();

		void currentWebSeedChanged (const QModelIndex&);
		void handleRemoveWebSeed ();
	};
}
}
