/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_POSHUKU_COOKIESEDITDIALOG_H
#define PLUGINS_POSHUKU_COOKIESEDITDIALOG_H
#include <QDialog>
#include "ui_cookieseditdialog.h"

namespace LC
{
namespace Poshuku
{
	class CookiesEditModel;
	class CookiesFilter;

	class CookiesEditDialog : public QDialog
	{
		Q_OBJECT

		Ui::CookiesEditDialog Ui_;
		CookiesEditModel *Model_;
		CookiesFilter *Filter_;
	public:
		CookiesEditDialog (QWidget* = 0);
	private slots:
		void handleClicked (const QModelIndex&);
		void handleAccepted ();
		void handleDomainChanged ();
		void handleNameChanged ();
		void on_Delete__released ();
	};
}
}

#endif
