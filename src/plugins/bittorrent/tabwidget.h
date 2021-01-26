/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include <QAction>
#include "ui_tabwidget.h"

namespace LC
{
namespace BitTorrent
{
	class SessionHolder;

	class TabWidget : public QWidget
	{
		Q_OBJECT

		SessionHolder& Holder_;

		Ui::TabWidget Ui_;
		bool TorrentSelectionChanged_ = false;
		Util::TagsCompleter *TagsChangeCompleter_;

		int Torrent_ = -1;
	public:
		explicit TabWidget (SessionHolder&, QWidget* = nullptr);

		int GetCurrentTorrent () const;
		void SetCurrentTorrent (int);
	public slots:
		void updateTorrentStats (const QModelIndex&, const QModelIndex&);
		void updateTorrentStats ();
	private:
		void UpdateDashboard ();
		void UpdateOverallStats ();
		void UpdateTorrentControl ();
	private slots:
		void on_OverallDownloadRateController__valueChanged (int);
		void on_OverallUploadRateController__valueChanged (int);
		void on_TorrentDownloadRateController__valueChanged (int);
		void on_TorrentUploadRateController__valueChanged (int);
		void on_TorrentManaged__clicked (bool);
		void on_TorrentSequentialDownload__clicked (bool);
		void on_DownloadingTorrents__valueChanged (int);
		void on_UploadingTorrents__valueChanged (int);
		void on_TorrentTags__editingFinished ();
	};
}
}
