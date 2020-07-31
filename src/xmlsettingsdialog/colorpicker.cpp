/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "colorpicker.h"
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QColorDialog>
#include <QApplication>
#include <QtDebug>

namespace LC
{
	ColorPicker::ColorPicker (const QString& title, QWidget *parent)
	: QWidget { parent }
	, Title_ { title.isEmpty () ? tr ("Choose color") : title }
	, Label_ { new QLabel { this } }
	, ChooseButton_ { new QPushButton { tr ("Choose...") } }
	{
		const auto lay = new QHBoxLayout;
		lay->setContentsMargins (0, 0, 0, 0);
		lay->addWidget (Label_);
		lay->addWidget (ChooseButton_);
		setLayout (lay);
		connect (ChooseButton_,
				SIGNAL (released ()),
				this,
				SLOT (chooseColor ()));
		Label_->setMinimumWidth (fontMetrics ().horizontalAdvance ("  #RRRRGGGGBBBB  "));
	}

	void ColorPicker::SetCurrentColor (const QColor& color)
	{
		Color_ = color;

		int height = QApplication::fontMetrics ().height ();
		int width = 1.62 * height;
		QPixmap pixmap { width, height };
		pixmap.fill (Color_);
		Label_->setPixmap (pixmap);
	}

	QColor ColorPicker::GetCurrentColor () const
	{
		return Color_;
	}

	void ColorPicker::chooseColor ()
	{
		const auto& color = QColorDialog::getColor (Color_,
				this,
				Title_);

		if (color == Color_ ||
				!color.isValid ())
			return;

		SetCurrentColor (color);
		emit currentColorChanged (Color_);
	}
}
