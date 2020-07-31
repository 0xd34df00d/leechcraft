/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_gwitemsremovaldialog.h"

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	class GlooxCLEntry;

	class GWItemsRemovalDialog : public QDialog
	{
		Ui::GWItemsRemovalDialog Ui_;
	public:
		GWItemsRemovalDialog (const QList<GlooxCLEntry*>&, QWidget* = 0);
	};
}
}
}
