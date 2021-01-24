/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QPersistentModelIndex>

class QSortFilterProxyModel;

namespace Ui
{
	class TorrentTabWidget;
}

namespace LC
{
namespace BitTorrent
{
	class PeersTabLinker : public QObject
	{
		Q_OBJECT

		Ui::TorrentTabWidget *Ui_;
	public:
		PeersTabLinker (Ui::TorrentTabWidget*, QObject* = nullptr);
	private slots:
		void handleNewRow (const QModelIndex&);
		void update ();
	};
}
}
