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

#include "colorpicker.h"
#include <QColorDialog>
#include <QApplication>
#include <QStyle>
#include <QPaintEvent>
#include <QPainter>
#include <QStyleOption>

namespace LeechCraft
{
	const int Padding_ = 6;
	const int DoublePadding_ = Padding_ * 2;
	
	ColorPicker::ColorPicker (const QString& title, QWidget *parent)
	: QPushButton (parent)
	, Title_ (title)
	{
		connect (this,
				SIGNAL (released ()),
				this,
				SLOT (chooseColor ()));
	}
	
	void ColorPicker::paintEvent (QPaintEvent *event)
	{
		QStyleOptionButton option;
		option.initFrom (this);
		option.state = isDown () ? QStyle::State_Sunken : QStyle::State_Raised;
		
		if (isDefault ())
			option.features |= QStyleOptionButton::DefaultButton;
		
		option.text = text ();
		option.icon = icon ();
		const QRect& rect = event->rect ();
		option.rect = rect;
		
		const QRect itemRect (rect.x () + Padding_, rect.y () + Padding_,
				rect.width () - DoublePadding_, rect.height () - DoublePadding_);
		
		
		const QRect colorRect (QPoint (0, 0), rect.size () - QSize (DoublePadding_, DoublePadding_));
		
		QPixmap pixmap (colorRect.size ());
		QPainter painter (&pixmap);
		
		painter.setRenderHint (QPainter::Antialiasing);
		
		painter.fillRect (colorRect, Color_);
		
		QPainter widgetPainter (this);
		QApplication::style ()->drawControl (QStyle::CE_PushButton, &option,
				&widgetPainter, this);
		widgetPainter.setRenderHint (QPainter::Antialiasing);
		widgetPainter.drawPixmap (itemRect, pixmap);
	}


	void ColorPicker::SetCurrentColor (const QColor& color)
	{
		Color_ = color;
		update ();
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

		if (color == Color_ ||
				!color.isValid ())
			return;

		SetCurrentColor (color);
		emit currentColorChanged (Color_);
	}
};

