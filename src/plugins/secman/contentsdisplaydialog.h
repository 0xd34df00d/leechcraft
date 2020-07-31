/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Vadim Misbakh-Soloviev, Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_contentsdisplaydialog.h"

class QStandardItemModel;

namespace LC
{
namespace SecMan
{
	class ContentsDisplayDialog : public QDialog
	{
		Q_OBJECT

		Ui::ContentsDisplayDialog Ui_;
		QStandardItemModel *ContentsModel_;
	public:
		ContentsDisplayDialog (QWidget* = 0);
	};
}
}
