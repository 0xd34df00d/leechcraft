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

#include "imagedialog.h"
#include <QFileDialog>
#include <QUrl>

namespace LeechCraft
{
namespace LHTR
{
	ImageDialog::ImageDialog (QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);
		on_TypeEmbed__toggled (Ui_.TypeEmbed_->isChecked ());
	}

	QString ImageDialog::GetPath () const
	{
		return Ui_.Path_->text ();
	}

	QString ImageDialog::GetAlt () const
	{
		return Ui_.Alt_->text ();
	}

	int ImageDialog::GetWidth () const
	{
		return Ui_.Width_->value ();
	}

	int ImageDialog::GetHeight () const
	{
		return Ui_.Height_->value ();
	}

	QString ImageDialog::GetFloat () const
	{
		switch (Ui_.Float_->currentIndex ())
		{
		case 1:
			return "left";
		case 2:
			return "right";
		default:
			return "none";
		}
	}

	void ImageDialog::on_TypeEmbed__toggled (bool on)
	{
		Ui_.Browse_->setEnabled (on);
	}

	void ImageDialog::on_Browse__released ()
	{
		const QString& path = QFileDialog::getOpenFileName (this,
				tr ("Select image"),
				QDir::homePath ());
		if (path.isEmpty ())
			return;

		Ui_.Path_->setText (QUrl::fromLocalFile (path).toString ());

		QImage image (path);
		if (image.isNull ())
		{
			Ui_.Width_->setValue (0);
			Ui_.Height_->setValue (0);
		}
		else
		{
			Ui_.Width_->setValue (image.width ());
			Ui_.Height_->setValue (image.height ());
		}
	}
}
}
