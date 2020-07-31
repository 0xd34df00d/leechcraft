/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include <interfaces/core/icoreproxyfwd.h>
#include "ui_searcherslist.h"

namespace LC
{
struct Entity;

namespace SeekThru
{
	class SearchersList : public QWidget
	{
		Q_OBJECT

		const ICoreProxy_ptr Proxy_;

		Ui::SearchersList Ui_;
		QModelIndex Current_;
	public:
		explicit SearchersList (const ICoreProxy_ptr&, QWidget* = nullptr);
	private slots:
		void handleCurrentChanged (const QModelIndex&);
		void on_ButtonAdd__released ();
		void on_ButtonRemove__released ();
		void on_Tags__editingFinished ();
	};
}
}
