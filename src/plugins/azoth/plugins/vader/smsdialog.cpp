/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "smsdialog.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Vader
{
	SMSDialog::SMSDialog (QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);
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
