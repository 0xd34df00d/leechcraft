/**********************************************************************
 *  LeechCraft - modular cross-platform feature rich internet client.
 *  Copyright (C) 2010-2013  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "dummytexteditor.h"
#include <QWebFrame>
#include <util/sll/unreachable.h>

namespace LC
{
namespace Blogique
{
	DummyTextEditor::DummyTextEditor (QWidget *parent)
	: QWebView (parent)
	{
		page ()->setContentEditable (true);
		connect (page (),
				SIGNAL (contentsChanged ()),
				this,
				SIGNAL (textChanged ()));
	}

	QString DummyTextEditor::GetContents (ContentType type) const
	{
		switch (type)
		{
		case ContentType::HTML:
			return page ()->mainFrame ()->toHtml ();
		case ContentType::PlainText:
			return page ()->mainFrame ()->toPlainText ();
		}

		Util::Unreachable ();
	}

	void DummyTextEditor::SetContents (QString contents, ContentType type)
	{
		switch (type)
		{
		case ContentType::HTML:
			setHtml (contents);
			break;
		case ContentType::PlainText:
			setContent (contents.toUtf8 ());
			break;
		}
	}

	void DummyTextEditor::AppendAction (QAction*)
	{
	}

	void DummyTextEditor::RemoveAction (QAction*)
	{
	}

	void DummyTextEditor::AppendSeparator ()
	{
	}

	QAction* DummyTextEditor::GetEditorAction (EditorAction)
	{
		return 0;
	}

	void DummyTextEditor::SetBackgroundColor (const QColor&, ContentType)
	{
	}

	QWidget* DummyTextEditor::GetWidget ()
	{
		return this;
	}

	QObject* DummyTextEditor::GetQObject ()
	{
		return this;
	}
}
}
