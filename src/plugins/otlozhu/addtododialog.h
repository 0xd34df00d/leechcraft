/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_addtododialog.h"
#include "todoitem.h"

namespace LC
{
namespace Otlozhu
{
	class AddTodoDialog : public QDialog
	{
		Ui::AddTodoDialog Ui_;
	public:
		AddTodoDialog (QWidget* = 0);

		TodoItem_ptr GetItem () const;
	private:
		QString GetTitle () const;
		QStringList GetTags () const;
	};
}
}
