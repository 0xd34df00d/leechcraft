/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QApplication>
#include <QMap>
#include "filepicker.h"

using namespace LC;

FilePicker::FilePicker (FilePicker::Type type, QWidget *parent)
: QWidget (parent)
, ClearOnCancel_ (false)
, Type_ (type)
{
	LineEdit_ = new QLineEdit (this);
	BrowseButton_ = new QPushButton (tr ("Browse..."));
	QHBoxLayout *lay = new QHBoxLayout;
	lay->setContentsMargins (0, 0, 0, 0);
	lay->addWidget (LineEdit_);
	lay->addWidget (BrowseButton_);
	setLayout (lay);
	connect (BrowseButton_,
			SIGNAL (released ()),
			this,
			SLOT (chooseFile ()));
	connect (LineEdit_,
			SIGNAL (textEdited (const QString&)),
			this,
			SIGNAL (textChanged (const QString&)));
	LineEdit_->setMinimumWidth (fontMetrics ().horizontalAdvance ("thisismaybeadefaultsettingstring,dont"));
}

void FilePicker::SetText (QString text)
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

void FilePicker::SetFilter (const QString& filter)
{
	Filter_ = filter;
}

void FilePicker::chooseFile ()
{
	QString name;
	switch (Type_)
	{
	case Type::ExistingDirectory:
		name = QFileDialog::getExistingDirectory (this,
				tr ("Select directory"),
				LineEdit_->text (),
				{});
		break;
	case Type::OpenFileName:
		name = QFileDialog::getOpenFileName (this,
				tr ("Select file"),
				LineEdit_->text (),
				Filter_);
		break;
	case Type::SaveFileName:
		name = QFileDialog::getSaveFileName (this,
				tr ("Select file"),
				LineEdit_->text (),
				Filter_);
		break;
	}
	if (name.isEmpty () && !ClearOnCancel_)
		return;

	LineEdit_->setText (name);
	emit textChanged (name);
}

