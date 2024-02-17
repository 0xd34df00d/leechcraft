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
	, ColorLabel_ { *new QLabel { this } }
	, NameLabel_ { *new QLabel { this } }
	{
		const auto lay = new QHBoxLayout;
		lay->setContentsMargins (0, 0, 0, 0);
		lay->addWidget (&ColorLabel_);
		lay->addWidget (&NameLabel_);
		const auto chooseButton = new QPushButton { tr ("Choose...") };
		lay->addWidget (chooseButton);
		setLayout (lay);

		connect (chooseButton,
				&QPushButton::released,
				this,
				[this]
				{
					const auto& color = QColorDialog::getColor (Color_, this, Title_);
					if (color == Color_ ||
							!color.isValid ())
						return;

					SetCurrentColor (color);
					emit currentColorChanged (Color_);
				});
	}

	void ColorPicker::SetCurrentColor (const QColor& color)
	{
		Color_ = color;

		NameLabel_.setText (color.name (QColor::HexRgb));

		QPixmap pixmap { GetPixmapSize () };
		pixmap.fill (Color_);
		ColorLabel_.setPixmap (pixmap);
	}

	QColor ColorPicker::GetCurrentColor () const
	{
		return Color_;
	}

	QSize ColorPicker::GetPixmapSize () const
	{
		auto height = fontMetrics ().height ();
		auto width = 1.62 * height;
		return { static_cast<int> (width), height };
	}
}
