/**********************************************************************
 *  LeechCraft - modular cross-platform feature rich internet client.
 *  Copyright (C) 2010-2013  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWebView>
#include <interfaces/itexteditor.h>

namespace LC
{
namespace Blogique
{
	class DummyTextEditor : public QWebView
						  , public IEditorWidget
	{
		Q_OBJECT
		Q_INTERFACES (IEditorWidget)
	public:
		DummyTextEditor (QWidget *parent = 0);

		QString GetContents (ContentType type) const;
		void SetContents (QString contents, ContentType type);

		void AppendAction (QAction* action);
		void RemoveAction (QAction* action);
		void AppendSeparator();

		QAction* GetEditorAction (EditorAction action);

		void SetBackgroundColor (const QColor& color, ContentType editor);

		QWidget* GetWidget ();
		QObject* GetQObject ();
	signals:
		void textChanged ();
	};
}
}
