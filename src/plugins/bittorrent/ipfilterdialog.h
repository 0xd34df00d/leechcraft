/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_ipfilterdialog.h"

namespace LC
{
namespace BitTorrent
{
	using BanRange_t = QPair<QString, QString>;

	class IPFilterDialog : public QDialog
	{
		Q_OBJECT

		Ui::IPFilterDialog Ui_;
	public:
		IPFilterDialog (QWidget* = 0);

		QList<QPair<BanRange_t, bool>> GetFilter () const;
	private slots:
		void on_Tree__currentItemChanged (QTreeWidgetItem*);
		void on_Tree__itemClicked (QTreeWidgetItem*, int);
		void on_Add__released ();
		void on_Modify__released ();
		void on_Remove__released ();
	};
}
}
