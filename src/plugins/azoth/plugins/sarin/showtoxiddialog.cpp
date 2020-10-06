/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "showtoxiddialog.h"
#include <QApplication>
#include <QClipboard>

namespace LC::Azoth::Sarin
{
	ShowToxIdDialog::ShowToxIdDialog (const QString& placeholder, QWidget *parent)
	: QDialog { parent }
	{
		Ui_.setupUi (this);
		Ui_.ToxIdEdit_->setPlaceholderText (placeholder);

		setToxId ({});
	}

	void ShowToxIdDialog::setToxId (const QString& id)
	{
		Ui_.ToxIdEdit_->setText (id);
		Ui_.CopyButton_->setEnabled (!id.isEmpty ());
	}

	void ShowToxIdDialog::on_CopyButton__released ()
	{
		const auto& id = Ui_.ToxIdEdit_->text ();
		QApplication::clipboard ()->setText (id, QClipboard::Clipboard);
	}
}
