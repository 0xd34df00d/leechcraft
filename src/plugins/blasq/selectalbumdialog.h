/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_selectalbumdialog.h"

namespace LC
{
namespace Blasq
{
	class IAccount;
	class CollectionsFilterModel;

	class SelectAlbumDialog : public QDialog
	{
		Q_OBJECT

		Ui::SelectAlbumDialog Ui_;

		CollectionsFilterModel * const Filter_;
		IAccount * const Acc_;
	public:
		SelectAlbumDialog (IAccount*, QWidget* = 0);

		QModelIndex GetSelectedCollection () const;
	private slots:
		void on_AddButton__released ();
	};
}
}
