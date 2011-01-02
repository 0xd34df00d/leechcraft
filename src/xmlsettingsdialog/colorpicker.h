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

#ifndef XMLSETTINGSDIALOG_COLORPICKER_H
#define XMLSETTINGSDIALOG_COLORPICKER_H
#include <QWidget>
#include <QColor>

class QLabel;
class QPushButton;

namespace LeechCraft
{
	class ColorPicker : public QWidget
	{
		Q_OBJECT

		QString Title_;
		QColor Color_;
		QLabel *Label_;
		QPushButton *ChooseButton_;
	public:
		ColorPicker (const QString& = QString (), QWidget* = 0);
		void SetCurrentColor (const QColor&);
		QColor GetCurrentColor () const;
	private slots:
		void chooseColor ();
	signals:
		void currentColorChanged (const QColor&);
	};
};

#endif

