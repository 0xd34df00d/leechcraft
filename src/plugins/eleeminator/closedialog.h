/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QDialog>
#include "ui_closedialog.h"

class QAbstractItemModel;

namespace LC::Eleeminator
{
	class CloseDialog : public QDialog
	{
		Ui::CloseDialog Ui_;

		const std::shared_ptr<QAbstractItemModel> Model_;
	public:
		CloseDialog (QAbstractItemModel*, QWidget* = nullptr);
	};
}
