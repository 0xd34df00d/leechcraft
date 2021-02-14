/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QTabWidget>
#include <QModelIndex>
#include "ui_torrenttabwidget.h"

class QAbstractItemModel;

namespace libtorrent
{
	class session;
}

namespace LC::BitTorrent
{
	class SessionSettingsManager;
	class PiecesModel;

	class TorrentTabWidget : public QTabWidget
	{
		Q_DECLARE_TR_FUNCTIONS (LC::BitTorrent::TorrentTabWidget)

		Ui::TorrentTabWidget Ui_;
		QAction *RemoveWebSeedAction_;

		QModelIndex Index_;
		QList<QModelIndex> SelectedIndices_;

		QAbstractItemModel *Model_ = nullptr;
		SessionSettingsManager *SSM_ = nullptr;
		libtorrent::session *Session_ = nullptr;

		std::unique_ptr<PiecesModel> PiecesModel_;
		std::unique_ptr<QAbstractItemModel> WebSeedsModel_;
	public:
		explicit TorrentTabWidget (QWidget* = nullptr);
		~TorrentTabWidget () override;

		struct Dependencies
		{
			AlertDispatcher& AlertDispatcher_;
			QAbstractItemModel& Model_;
			SessionSettingsManager& SSM_;
			libtorrent::session& Session_;
		};

		void SetDependencies (const Dependencies&);

		void SetChangeTrackersAction (QAction*);

		void SetCurrentIndex (const QModelIndex&);
		void SetSelectedIndices (const QList<QModelIndex>&);
		void InvalidateSelection ();
	private:
		void UpdateTorrentStats ();

		template<typename F>
		void ForEachSelected (F&&) const;

		void UpdateDashboard ();
		void UpdateOverallStats ();
		void UpdateTorrentControl ();

		void AddWebSeed ();
		void RemoveWebSeed ();

		void SetTabWidgetSettings ();
	};
}
