/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "fontpicker.h"
#include <QLabel>
#include <QFontInfo>
#include <QPushButton>
#include <QFontDialog>
#include <QHBoxLayout>
#include <QApplication>

namespace LC
{
	FontPicker::FontPicker (const QString& title, QWidget *parent)
	: QWidget (parent)
	, Title_ (title)
	{
		if (Title_.isEmpty ())
			Title_ = tr ("Choose font");
		Label_ = new QLabel (this);
		ChooseButton_ = new QPushButton (tr ("Choose..."));
		QHBoxLayout *lay = new QHBoxLayout;
		lay->setContentsMargins (0, 0, 0, 0);
		lay->addWidget (Label_);
		lay->addWidget (ChooseButton_);
		setLayout (lay);
		connect (ChooseButton_,
				SIGNAL (released ()),
				this,
				SLOT (chooseFont ()));

		const auto fontNameWidth = fontMetrics ().horizontalAdvance (QApplication::font ().toString ());
		Label_->setMinimumWidth (1.5 * fontNameWidth);
	}

	void FontPicker::SetCurrentFont (const QFont& font)
	{
		Font_ = font;
		QString text = Font_.family ();
		text += tr (", %1 pt")
			.arg (QFontInfo (Font_).pointSize ());
		if (Font_.bold ())
			text += tr (", bold");
		if (Font_.italic ())
			text += tr (", italic");
		if (Font_.underline ())
			text += tr (", underlined");
		if (Font_.strikeOut ())
			text += tr (", striken out");
		Label_->setText (text);
	}

	QFont FontPicker::GetCurrentFont () const
	{
		return Font_;
	}

	void FontPicker::chooseFont ()
	{
		bool ok = false;
		QFont font = QFontDialog::getFont (&ok,
				Font_,
				this,
				Title_);

		if (!ok ||
				font == Font_)
			return;

		SetCurrentFont (font);
		emit currentFontChanged (Font_);
	}
};

