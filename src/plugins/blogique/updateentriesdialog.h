/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_updateentriesdialog.h"

namespace LC
{
namespace Blogique
{
	class UpdateEntriesDialog : public QDialog
	{
		Q_OBJECT

		Ui::UpdateEntriesDialog Ui_;

	public:
		UpdateEntriesDialog (QWidget *parent = 0);

		int GetCount () const;

	public slots:
		void accept ();
	};
}
}
