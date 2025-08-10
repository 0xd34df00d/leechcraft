/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "ui_historywidget.h"

namespace LC::Util
{
	class FixedStringFilterProxyModel;
}

namespace LC
{
namespace Poshuku
{
	class HistoryFilterModel;

	class HistoryWidget : public QWidget
	{
		Q_OBJECT

		Ui::HistoryWidget Ui_;
		Util::FixedStringFilterProxyModel * const HistoryFilterModel_;
	public:
		HistoryWidget (QWidget* = 0);
	private slots:
		void on_HistoryView__activated (const QModelIndex&);
		void updateHistoryFilter ();
	};
}
}
