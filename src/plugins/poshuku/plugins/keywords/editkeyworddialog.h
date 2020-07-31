/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_editkeyworddialog.h"

namespace LC
{
namespace Poshuku
{
namespace Keywords
{ 
	class EditKeywordDialog : public QDialog
	{
		Ui::EditKeywordDialog Ui_;
	public:
		EditKeywordDialog (const QString& url, const QString& keyword, QWidget *parent = nullptr);

		QString GetUrl () const;
		QString GetKeyword () const;
	};
}
}
}
