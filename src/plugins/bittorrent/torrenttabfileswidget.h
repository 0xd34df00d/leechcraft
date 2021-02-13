/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include <QModelIndex>
#include "ui_torrenttabfileswidget.h"

class QSortFilterProxyModel;

namespace LC::BitTorrent
{
	class AlertDispatcher;
	class TorrentFilesModel;

	class TorrentTabFilesWidget : public QWidget
	{
		Q_DECLARE_TR_FUNCTIONS (LC::BitTorrent::TorrentTabFilesWidget)

		Ui::TorrentTabFilesWidget Ui_;
		QSortFilterProxyModel * const ProxyModel_;

		AlertDispatcher *AlertDispatcher_;

		TorrentFilesModel *CurrentFilesModel_ = nullptr;
	public:
		explicit TorrentTabFilesWidget (QWidget* = nullptr);

		void SetAlertDispatcher (AlertDispatcher&);

		void SetCurrentIndex (const QModelIndex&);
	private:
		QList<QModelIndex> GetSelectedIndexes () const;
		void HandleFileSelected (const QModelIndex&);
		void ShowContextMenu (const QPoint&);
	};
}
