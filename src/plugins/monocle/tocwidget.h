/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include "interfaces/monocle/ihavetoc.h"
#include "ui_tocwidget.h"

class QStandardItemModel;
class QStandardItem;

namespace LC
{
namespace Monocle
{
	class IPageLink;
	using IPageLink_ptr = std::shared_ptr<IPageLink>;

	class TOCWidget : public QWidget
	{
		Q_OBJECT

		Ui::TOCWidget Ui_;
		QStandardItemModel *Model_;

		QHash<QStandardItem*, ILink_ptr> Item2Link_;
		QHash<ILink_ptr, QStandardItem*> Link2Item_;
		QList<IPageLink_ptr> IntraDocPageLinks_;
	public:
		TOCWidget (QWidget* = 0);

		void SetTOC (const TOCEntryLevel_t&);
	private:
		template<typename T>
		void AddWorker (T, const TOCEntryLevel_t&);
	public slots:
		void updateCurrentPage (int);
	private slots:
		void on_TOCTree__activated (const QModelIndex&);
	};
}
}
