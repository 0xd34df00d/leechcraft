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

#include "msgformatterwidget.h"
#include <boost/bind.hpp>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QToolBar>
#include <QAction>

namespace LeechCraft
{
namespace Azoth
{
	MsgFormatterWidget::MsgFormatterWidget (QTextEdit *edit, QWidget *parent)
	: QWidget (parent)
	, Edit_ (edit)
	{
		setLayout (new QVBoxLayout ());
		QToolBar *toolbar = new QToolBar ();
		toolbar->setIconSize (QSize (16, 16));
		layout ()->addWidget (toolbar);
		
		FormatBold_ = toolbar->addAction (tr ("Bold"),
				this,
				SLOT (handleBold ()));
		FormatBold_->setCheckable (true);
		FormatBold_->setProperty ("ActionIcon", "format_text_bold");

		FormatItalic_ = toolbar->addAction (tr ("Italic"),
				this,
				SLOT (handleItalic ()));
		FormatItalic_->setCheckable (true);
		FormatItalic_->setProperty ("ActionIcon", "format_text_italic");

		FormatUnderline_ = toolbar->addAction (tr ("Underline"),
				this,
				SLOT (handleUnderline ()));
		FormatUnderline_->setCheckable (true);
		FormatUnderline_->setProperty ("ActionIcon", "format_text_underline");
		
		FormatStrikeThrough_ = toolbar->addAction (tr ("Strike through"),
				this,
				SLOT (handleStrikeThrough ()));
		FormatStrikeThrough_->setCheckable (true);
		FormatStrikeThrough_->setProperty ("ActionIcon", "format_text_strikethrough");
		
		connect (Edit_,
				SIGNAL (currentCharFormatChanged (const QTextCharFormat&)),
				this,
				SLOT (updateState (const QTextCharFormat&)));
	}
	
	void MsgFormatterWidget::CharFormatActor (boost::function<void (QTextCharFormat*)> format)
	{
		QTextCursor cursor = Edit_->textCursor ();
		if (cursor.hasSelection ())
		{
			QTextCharFormat fmt = cursor.charFormat ();
			format (&fmt);
			cursor.setCharFormat (fmt);
		}
		else
		{
			QTextCharFormat fmt = Edit_->currentCharFormat ();
			format (&fmt);
			Edit_->setCurrentCharFormat (fmt);
		}
	}
	
	void MsgFormatterWidget::handleBold ()
	{
		CharFormatActor (boost::bind (&QTextCharFormat::setFontWeight,
						_1,
						FormatBold_->isChecked () ? QFont::Bold : QFont::Normal));
	}
	
	void MsgFormatterWidget::handleItalic ()
	{
		CharFormatActor (boost::bind (&QTextCharFormat::setFontItalic,
						_1,
						FormatItalic_->isChecked ()));
	}
	
	void MsgFormatterWidget::handleUnderline ()
	{
		CharFormatActor (boost::bind (&QTextCharFormat::setFontUnderline,
						_1,
						FormatUnderline_->isChecked ()));
	}
	
	void MsgFormatterWidget::handleStrikeThrough ()
	{
		CharFormatActor (boost::bind (&QTextCharFormat::setFontStrikeOut,
						_1,
						FormatStrikeThrough_->isChecked ()));
	}
	
	void MsgFormatterWidget::updateState (const QTextCharFormat& fmt)
	{
		FormatBold_->setChecked (fmt.fontWeight () != QFont::Normal);
		FormatItalic_->setChecked (fmt.fontItalic ());
		FormatUnderline_->setChecked (fmt.fontUnderline ());
		FormatStrikeThrough_->setChecked (fmt.fontStrikeOut ());
	}
}
}
