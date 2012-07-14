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
namespace Astrality
{
	VCardDialog::VCardDialog (QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);
	}

	void VCardDialog::SetAvatar (const QImage& image)
	{
		if (!image.isNull ())
			Ui_.AvatarLabel_->setPixmap (QPixmap::fromImage (image));
	}

	void VCardDialog::SetInfoFields (const Tp::ContactInfoFieldList& list)
	{
		if (list.isEmpty ())
		{
			Ui_.InfoFields_->setPlainText (tr ("No info or protocol doesn't support info."));
			return;
		}

		QStringList strings;
		Q_FOREACH (const Tp::ContactInfoField& field, list)
			strings << field.fieldName + ": " + field.fieldValue.join ("; ");
		Ui_.InfoFields_->setPlainText (strings.join ("\n"));
	}
}
}
}
