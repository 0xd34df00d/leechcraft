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

namespace LC::BitTorrent
{
	class SessionSettingsManager;

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

		SessionSettingsManager *SSM_ = nullptr;
		SessionHolder *Holder_ = nullptr;
	public:
		explicit TorrentTabWidget (QWidget* = nullptr);

		struct Dependencies
		{
			SessionSettingsManager *SSM_;
			SessionHolder& Holder_;
		};

		void SetDependencies (const Dependencies&);

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
		void setTabWidgetSettings ();

		void on_LabelComment__linkActivated (const QString&);

		void handleAddWebSeed ();

		void currentWebSeedChanged (const QModelIndex&);
		void handleRemoveWebSeed ();
	};
}
