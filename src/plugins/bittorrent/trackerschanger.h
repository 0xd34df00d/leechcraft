/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include <libtorrent/torrent_info.hpp>
#include "ui_trackerschanger.h"

namespace LC
{
namespace BitTorrent
{
	class TrackersChanger : public QDialog
	{
		Q_OBJECT

		Ui::TrackersChanger Ui_;
	public:
		TrackersChanger (const std::vector<libtorrent::announce_entry>&, QWidget* = 0);

		std::vector<libtorrent::announce_entry> GetTrackers () const;
	private slots:
		void currentItemChanged (QTreeWidgetItem*);
		void on_ButtonAdd__released ();
		void on_ButtonModify__released ();
		void on_ButtonRemove__released ();
	};
}
}
