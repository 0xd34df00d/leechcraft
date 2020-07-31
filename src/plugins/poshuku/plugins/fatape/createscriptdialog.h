/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_createscriptdialog.h"

namespace LC
{
namespace Poshuku
{
namespace FatApe
{
	class CreateScriptDialog : public QDialog
	{
		Q_OBJECT

		Ui::CreateScriptDialog Ui_;
	public:
		CreateScriptDialog (QWidget* = nullptr);

		QString GetNamespace () const;
		QString GetName () const;
		QString GetDescription () const;
		QString GetAuthor () const;
	private slots:
		void checkValidity ();
	};
}
}
}
