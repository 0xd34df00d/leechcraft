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

#include "vcarddialog.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Vader
{
	VCardDialog::VCardDialog (QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);
	}

	void VCardDialog::SetAvatar (const QImage& avatar)
	{
		if (!avatar.isNull ())
			Ui_.AvatarLabel_->setPixmap (QPixmap::fromImage (avatar));
	}

	void VCardDialog::SetInfo (QMap<QString, QString> headers)
	{
		Ui_.InfoFields_->clear ();

		QStringList strings;

		auto append = [&strings, &headers] (const QString& key, const QString& trPattern)
		{
			const auto& val = headers [key];
			if (!val.isEmpty ())
				strings << trPattern.arg (val);
		};
		append ("Nickname", tr ("Nickname: %1."));
		append ("FirstName", tr ("First name: %1."));
		append ("LastName", tr ("Last name: %1."));

		if (headers.contains ("Sex"))
			headers ["Sex"] = headers ["Sex"] == "1" ? tr ("male") : tr ("female");
		append ("Sex", tr ("Gender: %1."));

		append ("Birthday", tr ("Birthday: %1."));
		append ("Zodiac", tr ("Zodiac sign: %1."));
		append ("Location", tr ("Location: %1."));
		append ("Phone", tr ("Phone number: %1."));

		Ui_.InfoFields_->setPlainText (strings.join ("\n"));
	}
}
}
}
