/**********************************************************************
 *  LeechCraft - modular cross-platform feature rich internet client.
 *  Copyright (C) 2010-2013  Oleg Linkin <MaledicutsDeMagog@gmail.com>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "addcommentdialog.h"
#include <QMessageBox>
#include <QPushButton>
#include <interfaces/core/iiconthememanager.h>
#include "core.h"

namespace LC
{
namespace Blogique
{
	AddCommentDialog::AddCommentDialog (QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);
		SendButton_ = Ui_.ButtonBox_->addButton (tr ("Send"), QDialogButtonBox::AcceptRole);
		SendButton_->setIcon (Core::Instance ().GetCoreProxy ()->
					GetIconThemeManager ()->GetIcon ("mail-send"));
		on_CommentBody__textChanged ();
	}

	QString AddCommentDialog::GetSubject () const
	{
		return Ui_.CommentSubject_->text ();
	}

	QString AddCommentDialog::GetText () const
	{
		return Ui_.CommentBody_->toPlainText ();
	}

	void AddCommentDialog::on_CommentBody__textChanged ()
	{
		 SendButton_->setEnabled (!Ui_.CommentBody_->toPlainText ().isEmpty ());
	}
}
}
