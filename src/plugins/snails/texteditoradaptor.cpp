/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "texteditoradaptor.h"
#include <QTextEdit>

namespace LC
{
namespace Snails
{
	TextEditorAdaptor::TextEditorAdaptor (QTextEdit *edit)
	: QObject { edit }
	, Edit_ { edit }
	{
		connect (Edit_,
				SIGNAL (textChanged ()),
				this,
				SIGNAL (textChanged ()));
	}

	QString TextEditorAdaptor::GetContents (ContentType type) const
	{
		switch (type)
		{
		case ContentType::PlainText:
			return Edit_->toPlainText ();
		default:
			return {};
		}
	}

	void TextEditorAdaptor::SetContents (QString contents, ContentType type)
	{
		if (type == ContentType::PlainText)
			Edit_->setPlainText (contents);
	}

	QAction* TextEditorAdaptor::GetEditorAction (EditorAction)
	{
		return nullptr;
	}

	void TextEditorAdaptor::AppendAction (QAction*)
	{
	}

	void TextEditorAdaptor::AppendSeparator ()
	{
	}

	void TextEditorAdaptor::RemoveAction (QAction*)
	{
	}

	void TextEditorAdaptor::SetBackgroundColor (const QColor&, ContentType)
	{
	}

	QWidget* TextEditorAdaptor::GetWidget ()
	{
		return Edit_;
	}

	QObject* TextEditorAdaptor::GetQObject ()
	{
		return this;
	}

	bool TextEditorAdaptor::FindText (const QString& text)
	{
		return Edit_->find (text);
	}

	void TextEditorAdaptor::DeleteSelection ()
	{
		Edit_->textCursor ().deleteChar ();
	}

	void TextEditorAdaptor::SetFontFamily (FontFamily family, const QFont& font)
	{
		if (family != FontFamily::FixedFont)
			return;

		auto cursor = Edit_->textCursor ();
		cursor.select (QTextCursor::Document);

		auto fmt = cursor.charFormat ();

		auto newFont = font;
		newFont.setPixelSize (fmt.font ().pixelSize ());
		fmt.setFont (newFont);

		cursor.setCharFormat (fmt);
	}

	void TextEditorAdaptor::SetFontSize (FontSize type, int size)
	{
		if (type != FontSize::DefaultFixedFontSize)
			return;

		auto cursor = Edit_->textCursor ();
		cursor.select (QTextCursor::Document);

		auto fmt = cursor.charFormat ();

		auto font = fmt.font ();
		font.setPixelSize (size);
		fmt.setFont (font);

		cursor.setCharFormat (fmt);
	}
}
}
