/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011-2012  Minh Ngo
 * Copyright (C) 2006-2012 Georg Rudoy
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

#include "positionslider.h"
#include <QMouseEvent>
#include <QStyle>

namespace LeechCraft
{
namespace Laure
{
	const QString unFocusedStyle =
			"QSlider::sub-page:horizontal {"
				"border: 1px solid #999999;"
				"height: 8px;"
				"background: palette(highlight);"
				"margin: 2px 0;"
			"}"
			"QSlider::groove:horizontal {"
				"border: 1px solid #999999;"
				"height: 8px;"
				"background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
					"stop:0 palette(midlight), stop:1 palette(button));"
				"margin: 2px 0;"
			"}";
			
	const QString focusedStyle = unFocusedStyle +
			"QSlider::handle:horizontal {"
				"background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
					"stop:0 palette(button), stop:1 palette(midlight));"
				"border: 1px solid #777;"
				"width: 10px;"
				"height: 11px;"
				"margin-top: -2px;"
				"margin-bottom: -2px;"
				"border-radius: 4px;"
			"}";
	
	PositionSlider::PositionSlider (QWidget *parent)
	: QSlider (parent)
	{
		setMouseTracking (true);
	}
	
	bool PositionSlider::event (QEvent* e)
	{
		switch (e->type ())
		{
		case QEvent::HoverLeave:
			setStyleSheet (unFocusedStyle);
			break;
		case QEvent::HoverEnter:
			setStyleSheet (focusedStyle);
			break;
		}
		return QSlider::event (e);
	}
	
	void PositionSlider::mousePressEvent (QMouseEvent *ev)
	{
		emit sliderMoved (QStyle::sliderValueFromPosition (minimum (),
				maximum (), ev->pos ().x (), width ()));
	}
}
}