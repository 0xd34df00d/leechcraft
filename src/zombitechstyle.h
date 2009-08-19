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

#ifndef ZOMBITECHSTYLE_H
#define ZOMBITECHSTYLE_H
#include <QPlastiqueStyle>

class ZombiTechStyle : public QPlastiqueStyle
{
	Q_OBJECT

	QColor White_;
	QColor MidOrange_;
	QColor TextOrange_;
	QColor DarkOrange_;
	QColor Background_;
	QColor LightOrange_;
	QColor BorderOrange_;
	QColor DarkTextOrange_;
public:
	ZombiTechStyle ();
	virtual ~ZombiTechStyle ();

	virtual void polish (QPalette&);
	virtual void polish (QWidget*);
	virtual void unpolish (QWidget*);
	virtual int pixelMetric (PixelMetric, const QStyleOption*,
			const QWidget*) const;
	virtual int styleHint (StyleHint, const QStyleOption*,
			const QWidget*, QStyleHintReturn*) const;
	virtual void drawPrimitive (PrimitiveElement,
			const QStyleOption*, QPainter*, const QWidget*) const;
};

#endif

