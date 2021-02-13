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
#include <QCoreApplication>
#include <QModelIndex>
#include "ui_tabwidget.h"

namespace libtorrent
{
	class session;
}

namespace LC::BitTorrent
{
	class SessionSettingsManager;

	class TabWidget : public QWidget
	{
		Q_DECLARE_TR_FUNCTIONS (LC::BitTorrent::TabWidget)

		libtorrent::session& Session_;
		SessionSettingsManager& SSM_;

		Ui::TabWidget Ui_;

		QAbstractItemModel& Model_;

		QModelIndex Torrent_;
	public:
		explicit TabWidget (QAbstractItemModel&, libtorrent::session&, SessionSettingsManager&, QWidget* = nullptr);

		QModelIndex GetCurrentTorrent () const;
		void SetCurrentTorrent (const QModelIndex&);
		void UpdateTorrentStats ();
	private:
		void UpdateDashboard ();
		void UpdateOverallStats ();
		void UpdateTorrentControl ();
	};
}
