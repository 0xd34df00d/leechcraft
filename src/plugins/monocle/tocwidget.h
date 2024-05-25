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

namespace LC::Monocle
{
	class TOCWidget : public QWidget
	{
		Q_OBJECT

		Ui::TOCWidget Ui_;
		QStandardItemModel *Model_;

		QHash<QStandardItem*, NavigationAction> Item2Link_;
		QMap<NavigationAction, QStandardItem*> Link2Item_;
	public:
		explicit TOCWidget (QWidget* = nullptr);

		void SetTOC (const TOCEntryLevel_t&);

		void SetCurrentPage (int);
	private:
		template<typename T>
		void AddWorker (T, const TOCEntryLevel_t&);
	signals:
		void navigationRequested (const NavigationAction&);
	};
}
