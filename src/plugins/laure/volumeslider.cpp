/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011-2012  Minh Ngo
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

#include "volumeslider.h"
#include <QMouseEvent>
#include <QPainter>
#include <QTimer>
#include <QStyle>

namespace LeechCraft
{
namespace Laure
{
	VolumeSlider::VolumeSlider (QWidget *parent)
	: QSlider (parent)
	, VolumeSliderInset_ (QPixmap (":/plugins/laure/resources/img/volumeslider-inset.png"))
	{
		GenerateGradient ();
	}

	void VolumeSlider::GenerateGradient ()
	{
		const QImage mask (":/plugins/laure/resources/img/volumeslider-gradient.png");
		QImage gradient_image (mask.size (),
				QImage::Format_ARGB32_Premultiplied);
		QPainter p (&gradient_image);

		QLinearGradient gradient (gradient_image.rect ().topLeft (),
				gradient_image.rect ().topRight ());
		gradient.setColorAt (0, palette ().color (QPalette::Background));
		gradient.setColorAt (1, palette ().color (QPalette::Highlight));
		p.fillRect (gradient_image.rect (), QBrush (gradient));

		p.setCompositionMode (QPainter::CompositionMode_DestinationIn);
		p.drawImage (0, 0, mask);
		p.end ();

		VolumeSliderGradient_ = QPixmap::fromImage (gradient_image);
	}
	
	void VolumeSlider::paintEvent (QPaintEvent* ev)
	{
		const int padding = 7;
		const int offset = double ((width () - 2 * padding) * value ()) / maximum ();

		QPainter p (this);
		p.drawPixmap(0, 0, VolumeSliderGradient_, 0, 0, offset + padding, 0);
		p.drawPixmap (0, 0,  VolumeSliderInset_);
		
		p.setPen (palette ().color (QPalette::Disabled, QPalette::Text).dark ());
		QFont font;
		font.setPixelSize (9);
		p.setFont (font);
		const QRect rect (0, 0, 34, 15);
		p.drawText (rect, Qt::AlignRight | Qt::AlignVCenter, QString::number (value ()) + '%');
	}
	
	void VolumeSlider::mousePressEvent (QMouseEvent *ev)
	{
		if (ev->button () != Qt::RightButton)
			QSlider::setValue (QStyle::sliderValueFromPosition(minimum (),
							maximum (), ev->pos ().x (), width () - 2));
	}
	
	void VolumeSlider::mouseMoveEvent (QMouseEvent *ev)
	{
		mousePressEvent (ev);
	}
}
}
