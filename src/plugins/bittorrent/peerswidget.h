/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QWidget>
#include "ui_peerswidget.h"

class QSortFilterProxyModel;

namespace libtorrent
{
	class session;
}

namespace LC::BitTorrent
{
	class PeersModel;

	class PeersWidget : public QWidget
	{
		Ui::PeersWidget Ui_;

		libtorrent::session *Session_ = nullptr;

		std::unique_ptr<PeersModel> CurrentModel_;
		QSortFilterProxyModel * const PeersSorter_;
		QModelIndex TorrentIdx_;
	public:
		explicit PeersWidget (QWidget* = nullptr);
		~PeersWidget () override;

		void SetSession (libtorrent::session&);

		void SetSelectedTorrent (const QModelIndex&);
		void Update ();
	private:
		void UpdateDetails ();
	};
}
