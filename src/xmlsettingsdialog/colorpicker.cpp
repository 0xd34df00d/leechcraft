/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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
#include <QColorDialog>

namespace LeechCraft
{
	ColorPicker::ColorPicker (const QString& title, QWidget *parent)
	: QPushButton (parent)
	, Title_ (title)
	{
		if (Title_.isEmpty ())
			Title_ = tr ("Choose color");
		setAutoFillBackground (true);
		setFlat (true);
		connect (this,
				SIGNAL (released ()),
				this,
				SLOT (chooseColor ()));
	}

	void ColorPicker::SetCurrentColor (const QColor& color)
	{
		QPalette pal = palette ();
		pal.setColor (QPalette::Button, color);
		setPalette (pal);
	}

	QColor ColorPicker::GetCurrentColor () const
	{
		return Color_;
	}

	void ColorPicker::chooseColor ()
	{
		const QColor& color = QColorDialog::getColor (Color_,
				this,
				Title_);

		if (color == Color_ || !color.isValid ())
			return;

		SetCurrentColor (color);
		emit currentColorChanged (Color_);
	}
};

