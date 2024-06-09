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
#include "ui_searchtabwidget.h"

class QStandardItemModel;
class QStandardItem;

namespace LC::Monocle
{
	class TextSearchHandler;
	struct TextSearchHandlerResults;

	class SearchTabWidget : public QWidget
	{
		Q_OBJECT

		Ui::SearchTabWidget Ui_;

		QStandardItemModel& Model_;
		TextSearchHandler& SearchHandler_;

		QMap<QStandardItem*, TextSearchHandlerResults> Root2Results_;
	public:
		explicit SearchTabWidget (TextSearchHandler&, QWidget* = nullptr);

		void Reset ();
	private:
		void AddSearchResults (const TextSearchHandlerResults&);
		void SetResultsFromHistory (const QModelIndex&);
	};
}
