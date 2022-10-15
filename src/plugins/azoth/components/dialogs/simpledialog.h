/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_SIMPLEDIALOG_H
#define PLUGINS_AZOTH_SIMPLEDIALOG_H
#include <QDialog>
#include "ui_simpledialog.h"

namespace LC
{
namespace Azoth
{
	class SimpleDialog : public QDialog
	{
		Q_OBJECT
		
		Ui::SimpleDialog Ui_;
	public:
		SimpleDialog (QWidget* = 0);
		
		void SetWidget (QWidget*);
	};
}
}

#endif
