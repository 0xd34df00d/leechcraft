/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2012  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_collectionstatsdialog.h"

namespace LC
{
namespace LMP
{
	class CollectionStatsDialog : public QDialog
	{
		Q_OBJECT

		Ui::CollectionStatsDialog Ui_;
	public:
		CollectionStatsDialog (QWidget* = 0);
	protected:
		void keyReleaseEvent (QKeyEvent*);
	};
}
}
