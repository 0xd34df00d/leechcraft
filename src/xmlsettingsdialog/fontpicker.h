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

#ifndef XMLSETTINGSDIALOG_FONTPICKER_H
#define XMLSETTINGSDIALOG_FONTPICKER_H
#include <QWidget>
#include <QFont>

class QLabel;
class QPushButton;

namespace LeechCraft
{
	class FontPicker : public QWidget
	{
		Q_OBJECT

		QString Title_;
		QFont Font_;
		QLabel *Label_;
		QPushButton *ChooseButton_;
	public:
		FontPicker (const QString& = QString (), QWidget* = 0);
		void SetCurrentFont (const QFont&);
		QFont GetCurrentFont () const;
	private slots:
		void chooseFont ();
	signals:
		void currentFontChanged (const QFont&);
	};
};

#endif

