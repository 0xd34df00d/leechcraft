/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "filepicker.h"
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QFileDialog>
#include <util/sll/qtutil.h>

namespace LC
{
	FilePicker::FilePicker (FilePicker::Type type, QWidget *parent)
	: QWidget { parent }
	, LineEdit_ { *new QLineEdit }
	, BrowseButton_ { *new QPushButton (tr ("Browse...")) }
	, Type_ { type }
	{
		const auto lay = new QHBoxLayout;
		lay->setContentsMargins (0, 0, 0, 0);
		lay->addWidget (&LineEdit_);
		lay->addWidget (&BrowseButton_);
		setLayout (lay);

		connect (&BrowseButton_,
				&QPushButton::released,
				this,
				&FilePicker::ChooseFile);
		connect (&LineEdit_,
				&QLineEdit::textChanged,
				this,
				&FilePicker::textChanged);
		LineEdit_.setMinimumWidth (fontMetrics ().horizontalAdvance ("thisismaybeadefaultsettingstring,dont"_qs));
	}

	void FilePicker::SetText (const QString& text)
	{
		LineEdit_.setText (text);
	}

	QString FilePicker::GetText () const
	{
		return LineEdit_.text ();
	}

	void FilePicker::SetClearOnCancel (bool clear)
	{
		ClearOnCancel_ = clear;
	}

	void FilePicker::SetFilter (const QString& filter)
	{
		Filter_ = filter;
	}

	void FilePicker::ChooseFile ()
	{
		QString name;
		switch (Type_)
		{
		case Type::ExistingDirectory:
			name = QFileDialog::getExistingDirectory (this,
					tr ("Select directory"),
					LineEdit_.text (),
					{});
			break;
		case Type::OpenFileName:
			name = QFileDialog::getOpenFileName (this,
					tr ("Select file"),
					LineEdit_.text (),
					Filter_);
			break;
		case Type::SaveFileName:
			name = QFileDialog::getSaveFileName (this,
					tr ("Select file"),
					LineEdit_.text (),
					Filter_);
			break;
		}
		if (name.isEmpty () && !ClearOnCancel_)
			return;

		LineEdit_.setText (name);
		emit textChanged (name);
	}
}
