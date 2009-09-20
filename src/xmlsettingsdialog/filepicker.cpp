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

#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QApplication>
#include "filepicker.h"

using namespace LeechCraft;

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
	LineEdit_->setMinimumWidth (QApplication::fontMetrics ()
			.width ("thisismaybeadefaultsettingstring,dont"));
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

void FilePicker::SetFilter (const QString& filter)
{
	Filter_ = filter;
}

void FilePicker::chooseFile ()
{
	QString name;
	switch (Type_)
	{
		case TExistingDirectory:
			name = QFileDialog::getExistingDirectory (this,
					tr ("Select directory"),
					LineEdit_->text (),
					0);
			break;
		case TOpenFileName:
			name = QFileDialog::getOpenFileName (this,
					tr ("Select file"),
					LineEdit_->text (),
					Filter_);
			break;
		case TSaveFileName:
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

