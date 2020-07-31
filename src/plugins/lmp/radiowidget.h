/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include "ui_radiowidget.h"

class QStandardItem;
class QSortFilterProxyModel;

namespace LC
{
namespace LMP
{
	class Player;

	class RadioWidget : public QWidget
	{
		Q_OBJECT

		Ui::RadioWidget Ui_;

		Player *Player_ = nullptr;
		QSortFilterProxyModel *StationsProxy_;
	public:
		RadioWidget (QWidget* = 0);

		void SetPlayer (Player*);
	private:
		void AddUrl (const QUrl&);
	private slots:
		void handleRefresh ();

		void handleAddUrl ();
		void handleAddCurrentUrl ();
		void handleRemoveUrl ();
		void handleDownloadTracks ();

		void on_StationsView__customContextMenuRequested (const QPoint&);
		void on_StationsView__doubleClicked (const QModelIndex&);
	};
}
}
