/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "smsdialog.h"

namespace LC
{
namespace Azoth
{
namespace Vader
{
	SMSDialog::SMSDialog (QString phone, QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);

		if (!phone.startsWith ('+'))
			phone.prepend ('+');
		Ui_.Phone_->setText (phone);
	}

	QString SMSDialog::GetPhone () const
	{
		return Ui_.Phone_->text ();
	}

	QString SMSDialog::GetText() const
	{
		return Ui_.Text_->toPlainText ();
	}

	void SMSDialog::on_Text__textChanged ()
	{
		const auto& text = GetText ();
		const int size = text.size ();

		bool isAllLatin = true;
		for (int i = 0; i < size; ++i)
		{
			const QChar& ch = text.at (i);
			if (!ch.isLetter ())
				continue;

			char latin = ch.toLatin1 ();
			if (('a' <= latin && latin <= 'z') ||
					('A' <= latin && latin <= 'Z'))
				continue;

			isAllLatin = false;
			break;
		}

		const int maxSize = isAllLatin ? 135 : 35;
		Ui_.Counter_->setText (QString ("%1/%2")
				.arg (size)
				.arg (maxSize));

		if (text.size () > maxSize)
		{
			Ui_.Text_->setPlainText (text.left (maxSize));
			Ui_.Text_->moveCursor (QTextCursor::End);
		}
	}
}
}
}
