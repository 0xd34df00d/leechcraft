/*
    Copyright (c) 2008 by Rudoy Georg <0xd34df00d@gmail.com>

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
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
	public:
		FilePicker (QWidget *parent = 0);
		void SetText (const QString&);
		QString GetText () const;
	private slots:
		void chooseFile ();
	signals:
		void textChanged (const QString&);
	};
};

#endif

