/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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
#include <gloox/vcard.h>

namespace LeechCraft
{
namespace Plugins
{
namespace Azoth
{
namespace Plugins
{
namespace Xoox
{
	VCardDialog::VCardDialog (QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);
		Ui_.EditBirthday_->setVisible (false);
	}

	void VCardDialog::UpdateInfo (const gloox::VCard *vcard)
	{
		setWindowTitle (tr ("VCard for %1")
					.arg (QString::fromUtf8 (vcard->nickname ().c_str ())));

		Ui_.EditRealName_->setText (GetName (vcard));
		Ui_.EditNick_->setText (QString::fromUtf8 (vcard->nickname ().c_str ()));
		QDate date = QDate::fromString (QString::fromUtf8 (vcard->bday ().c_str ()), Qt::ISODate);
		if (date.isValid ())
			Ui_.EditBirthday_->setDate (date);
		Ui_.EditBirthday_->setVisible (date.isValid ());

		QStringList phones;
		Q_FOREACH (const gloox::VCard::Telephone& phone, const_cast<gloox::VCard*> (vcard)->telephone ())
			phones << QString::fromUtf8 (phone.number.c_str ());
		Ui_.EditPhone_->setText (phones.join ("; "));

		Ui_.EditURL_->setText (QString::fromUtf8 (vcard->url ().c_str ()));

		const gloox::VCard::Photo& photo = vcard->photo ();
		QByteArray data (photo.binval.c_str(), photo.binval.size ());

		QPixmap px = QPixmap::fromImage (QImage::fromData (data));
		if (!px.isNull ())
		{
			const QSize& maxPx = Ui_.LabelPhoto_->maximumSize ();
			if (px.width () > maxPx.width () ||
					px.height () > maxPx.height ())
				px = px.scaled (maxPx, Qt::KeepAspectRatio, Qt::SmoothTransformation);
			Ui_.LabelPhoto_->setPixmap (px);
		}
		else
			Ui_.LabelPhoto_->setText (tr ("No photo"));
	}

	QString VCardDialog::GetName (const gloox::VCard *vcard)
	{
		const gloox::VCard::Name& name = vcard->name ();
		QString result;
		if (name.prefix.size ())
			result += QString (" ") + QString::fromUtf8 (name.prefix.c_str ());
		if (name.family.size ())
			result += QString (" ") + QString::fromUtf8 (name.family.c_str ());
		if (name.given.size ())
			result += QString (" ") + QString::fromUtf8 (name.given.c_str ());
		if (name.middle.size ())
			result += QString (" ") + QString::fromUtf8 (name.middle.c_str ());
		if (name.suffix.size ())
			result += QString (" ") + QString::fromUtf8 (name.suffix.c_str ());
		return result;
	}
}
}
}
}
}
