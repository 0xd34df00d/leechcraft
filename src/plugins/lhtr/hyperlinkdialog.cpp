/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "hyperlinkdialog.h"
#include <QPushButton>
#include <QtDebug>

namespace LC::LHTR
{
	HyperlinkDialog::HyperlinkDialog (QWidget *parent)
	: QDialog { parent }
	{
		Ui_.setupUi (this);

		const auto checkCanAccept = [this]
		{
			const bool can = !GetLink ().isEmpty () && !GetText ().isEmpty ();
			Ui_.ButtonBox_->button (QDialogButtonBox::Ok)->setEnabled (can);
		};
		connect (Ui_.Link_,
				&QLineEdit::textChanged,
				checkCanAccept);
		connect (Ui_.Text_,
				&QLineEdit::textChanged,
				checkCanAccept);
		checkCanAccept ();
	}

	QString HyperlinkDialog::GetLink () const
	{
		return Ui_.Link_->text ();
	}

	QString HyperlinkDialog::GetText () const
	{
		return Ui_.Text_->text ();
	}

	QString HyperlinkDialog::GetTitle () const
	{
		return Ui_.Title_->text ();
	}

	QString HyperlinkDialog::GetTarget () const
	{
		return Ui_.Target_->currentText ();
	}
}
