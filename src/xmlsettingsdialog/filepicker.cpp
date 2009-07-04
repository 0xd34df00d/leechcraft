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

#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QApplication>
#include "filepicker.h"

using namespace LeechCraft;

FilePicker::FilePicker (QWidget *parent)
: QWidget (parent)
, ClearOnCancel_ (false)
{
	LineEdit_ = new QLineEdit (this);
	BrowseButton_ = new QPushButton (tr ("Browse..."));
	QHBoxLayout *lay = new QHBoxLayout;
	lay->setContentsMargins (0, 0, 0, 0);
	lay->addWidget (LineEdit_);
	lay->addWidget (BrowseButton_);
	setLayout (lay);
	connect (BrowseButton_, SIGNAL (released ()), this, SLOT (chooseFile ()));
	LineEdit_->setMinimumWidth (QApplication::fontMetrics ().width ("thisismaybeadefaultsettingstring,dont"));
}

void FilePicker::SetText (const QString& text)
{
	LineEdit_->setText (text);
}

QString FilePicker::GetText () const
{
	return LineEdit_->text ();
}

void FilePicker::SetClearOnCancel (bool clear)
{
	ClearOnCancel_ = clear;
}

void FilePicker::chooseFile ()
{
	QString name = QFileDialog::getExistingDirectory (this, tr ("Select file"), LineEdit_->text (), 0);
	if (name.isEmpty () && !ClearOnCancel_)
		return;

	LineEdit_->setText (name);
	emit textChanged (name);
}

