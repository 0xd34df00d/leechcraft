/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "colorbutton.h"
#include <QColorDialog>

namespace LC::Util
{
	ColorButton::ColorButton (QWidget *parent)
	: QPushButton { parent }
	{
		connect (this,
				SIGNAL (released ()),
				this,
				SLOT (handleSelector ()));

		SetColor (Qt::black);
	}

	QColor ColorButton::GetColor () const
	{
		return Color_;
	}

	void ColorButton::SetColor (const QColor& color)
	{
		if (Color_ == color)
			return;

		Color_ = color;
		emit colorChanged (Color_);

		QPixmap px { iconSize () };
		px.fill (Color_);
		setIcon (px);
	}

	void ColorButton::handleSelector ()
	{
		const auto color = QColorDialog::getColor (Color_,
				this,
				tr ("Select color"),
				QColorDialog::ShowAlphaChannel);
		if (!color.isValid ())
			return;

		SetColor (color);
	}
}
