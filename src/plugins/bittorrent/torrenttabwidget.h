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
#include "ui_torrenttabwidget.h"

class QAbstractItemModel;

namespace LC::BitTorrent
{
	class SessionSettingsManager;
	class PiecesModel;

	class TorrentTabWidget : public QTabWidget
	{
		Q_OBJECT

		Ui::TorrentTabWidget Ui_;
		QAction *RemoveWebSeedAction_;
		int Index_ = -1;
		QList<int> SelectedIndices_;

		SessionSettingsManager *SSM_ = nullptr;
		SessionHolder *Holder_ = nullptr;

		std::unique_ptr<PiecesModel> PiecesModel_;
		std::unique_ptr<QAbstractItemModel> WebSeedsModel_;
	public:
		explicit TorrentTabWidget (QWidget* = nullptr);
		~TorrentTabWidget () override;

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
