/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "vcarddialog.h"

namespace LC
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
