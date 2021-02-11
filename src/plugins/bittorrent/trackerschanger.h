/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include <QCoreApplication>
#include "ui_trackerschanger.h"

namespace libtorrent
{
	class announce_entry;
}

namespace LC::BitTorrent
{
	class TrackersChanger : public QDialog
	{
		Q_DECLARE_TR_FUNCTIONS (LC::BitTorrent::TrackersChanger)

		Ui::TrackersChanger Ui_;
	public:
		explicit TrackersChanger (const std::vector<libtorrent::announce_entry>&, QWidget* = nullptr);

		std::vector<libtorrent::announce_entry> GetTrackers () const;
	private slots:
		void AddTracker ();
		void ModifyTracker ();
		void RemoveTracker ();
	};
}
