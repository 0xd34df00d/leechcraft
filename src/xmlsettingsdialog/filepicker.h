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

/*
	Copyright (c) 2008 by Rudoy Georg <0xd34df00d@gmail.com>

 ***************************************************************************
 *																		 *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or	 *
 *   (at your option) any later version.								   *
 *																		 *
 ***************************************************************************
*/
#ifndef XMLSETTINGSDIALOG_FILEPICKER_H
#define XMLSETTINGSDIALOG_FILEPICKER_H
#include <QWidget>

class QLineEdit;
class QPushButton;

namespace LeechCraft
{
	class FilePicker : public QWidget
	{
		Q_OBJECT

		QLineEdit *LineEdit_;
		QPushButton *BrowseButton_;
		bool ClearOnCancel_;
	public:
		FilePicker (QWidget *parent = 0);
		void SetText (const QString&);
		QString GetText () const;
		void SetClearOnCancel (bool);
	private slots:
		void chooseFile ();
	signals:
		void textChanged (const QString&);
	};
};

#endif

