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

namespace LC
{
namespace Monocle
{
	class TextSearchHandler;
	struct TextSearchHandlerResults;

	class IDocument;
	typedef std::shared_ptr<IDocument> IDocument_ptr;

	class SearchTabWidget : public QWidget
	{
		Q_OBJECT

		Ui::SearchTabWidget Ui_;

		QStandardItemModel * const Model_;
		TextSearchHandler * const SearchHandler_;

		QMap<QStandardItem*, TextSearchHandlerResults> Root2Results_;
	public:
		SearchTabWidget (TextSearchHandler*, QWidget* = nullptr);

		void HandleDoc (const IDocument_ptr&);
	private slots:
		void handleSearchResults (const TextSearchHandlerResults&);
		void on_ResultsTree__activated (const QModelIndex&);
	};
}
}
