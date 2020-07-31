/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include <interfaces/iinfo.h>
#include "ui_searchtext.h"

namespace LC
{
namespace Poshuku
{
	class SearchText : public QDialog
	{
		Q_OBJECT

		const ICoreProxy_ptr Proxy_;
		const QString Text_;

		Ui::SearchText Ui_;
	public:
		SearchText (const QString&, const ICoreProxy_ptr&, QWidget* = nullptr);
	private:
		void DoSearch ();
	private slots:
		void on_MarkAll__released ();
		void on_UnmarkAll__released ();
	};
}
}
