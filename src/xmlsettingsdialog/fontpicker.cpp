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

#include "fontpicker.h"
#include <QLabel>
#include <QFontInfo>
#include <QPushButton>
#include <QFontDialog>
#include <QHBoxLayout>
#include <QApplication>

namespace LeechCraft
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
		Label_->setMinimumWidth (1.5 * QApplication::fontMetrics ()
				.width (QApplication::font ().toString ()));
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

