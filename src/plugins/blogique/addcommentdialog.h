/**********************************************************************
 *  LeechCraft - modular cross-platform feature rich internet client.
 *  Copyright (C) 2010-2013  Oleg Linkin <MaledicutsDeMagog@gmail.com>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_addcommentdialog.h"

class QPushButton;

namespace LC
{
namespace Blogique
{
	class AddCommentDialog : public QDialog
	{
		Q_OBJECT

		Ui::AddCommentDialog Ui_;

		QPushButton *SendButton_;

	public:
		explicit AddCommentDialog (QWidget *parent = 0);

		QString GetSubject () const;
		QString GetText () const;

	private slots:
		void on_CommentBody__textChanged ();
	};
}
}
