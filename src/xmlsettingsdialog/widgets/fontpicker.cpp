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
#include <util/sll/qtutil.h>

namespace LC
{
	FontPicker::FontPicker (const QString& title, QWidget *parent)
	: QWidget { parent }
	, Title_ { title.isEmpty () ? tr ("Choose font") : title }
	, Label_ { *new QLabel }
	, ChooseButton_ { *new QPushButton { tr ("Choose") } }
	{
		const auto lay = new QHBoxLayout;
		lay->setContentsMargins (0, 0, 0, 0);
		lay->addWidget (&Label_);
		lay->addWidget (&ChooseButton_);
		setLayout (lay);
		connect (&ChooseButton_,
				&QPushButton::released,
				this,
				&FontPicker::ChooseFont);

		const auto fontNameWidth = fontMetrics ().horizontalAdvance (Label_.font ().toString ());
		Label_.setMinimumWidth (1.5 * fontNameWidth);
	}

	void FontPicker::SetCurrentFont (const QFont& font)
	{
		Font_ = font;

		QStringList props
		{
			Font_.family (),
			tr ("%1 pt").arg (QFontInfo { Font_ }.pointSize ()),
		};
		if (Font_.bold ())
			props << tr ("bold");
		if (Font_.italic ())
			props << tr ("italic");
		if (Font_.underline ())
			props << tr ("underlined");
		if (Font_.strikeOut ())
			props << tr ("striken out");
		Label_.setText (props.join (", "_qs));
	}

	QFont FontPicker::GetCurrentFont () const
	{
		return Font_;
	}

	void FontPicker::ChooseFont ()
	{
		bool ok = false;
		auto font = QFontDialog::getFont (&ok, Font_, this, Title_);
		if (!ok || font == Font_)
			return;

		SetCurrentFont (font);
		emit currentFontChanged (Font_);
	}
};

