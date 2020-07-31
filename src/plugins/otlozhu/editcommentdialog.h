/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_editcommentdialog.h"

namespace LC
{
namespace Otlozhu
{
	class EditCommentDialog : public QDialog
	{
		Q_OBJECT

		Ui::EditCommentDialog Ui_;
	public:
		EditCommentDialog (const QString& title,
				const QString& comment, QWidget* = 0);

		QString GetComment () const;
	};
}
}
