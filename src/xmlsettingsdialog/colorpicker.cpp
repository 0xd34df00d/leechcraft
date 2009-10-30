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

#include "colorpicker.h"
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QColorDialog>
#include <QApplication>
#include <QtDebug>

namespace LeechCraft
{
	ColorPicker::ColorPicker (const QString& title, QWidget *parent)
	: QWidget (parent)
	, Title_ (title)
	{
		if (Title_.isEmpty ())
			Title_ = tr ("Choose color");
		Label_ = new QLabel (this);
		ChooseButton_ = new QPushButton (tr ("Choose..."));
		QHBoxLayout *lay = new QHBoxLayout;
		lay->setContentsMargins (0, 0, 0, 0);
		lay->addWidget (Label_);
		lay->addWidget (ChooseButton_);
		setLayout (lay);
		connect (ChooseButton_,
				SIGNAL (released ()),
				this,
				SLOT (chooseColor ()));
		Label_->setMinimumWidth (QApplication::fontMetrics ()
				.width ("  #RRRRGGGGBBBB  "));
	}

	void ColorPicker::SetCurrentColor (const QColor& color)
	{
		Color_ = color;

		int height = QApplication::fontMetrics ().height ();
		int width = 1.62 * height;
		QPixmap pixmap (width, height);
		pixmap.fill (Color_);
		Label_->setPixmap (pixmap);
	}

	QColor ColorPicker::GetCurrentColor () const
	{
		return Color_;
	}

	void ColorPicker::chooseColor ()
	{
		QColor color = QColorDialog::getColor (Color_,
				this,
				Title_);

		if (color == Color_)
			return;

		SetCurrentColor (color);
		emit currentColorChanged (Color_);
	}
};

