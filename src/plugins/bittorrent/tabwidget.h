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

namespace LC::BitTorrent
{
	class SessionHolder;
	class SessionSettingsManager;

	class TabWidget : public QWidget
	{
		SessionHolder& Holder_;
		SessionSettingsManager& SSM_;

		Ui::TabWidget Ui_;

		int Torrent_ = -1;
	public:
		explicit TabWidget (SessionHolder&, SessionSettingsManager&, QWidget* = nullptr);

		int GetCurrentTorrent () const;
		void SetCurrentTorrent (int);
		void UpdateTorrentStats ();
	private:
		void UpdateDashboard ();
		void UpdateOverallStats ();
		void UpdateTorrentControl ();
	};
}
