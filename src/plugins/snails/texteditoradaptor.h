/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <interfaces/itexteditor.h>
#include <interfaces/iadvancedplaintexteditor.h>
#include <interfaces/iwkfontssettable.h>

class QTextEdit;

namespace LC
{
namespace Snails
{
	class TextEditorAdaptor : public QObject
							, public IEditorWidget
							, public IAdvancedPlainTextEditor
							, public IWkFontsSettable
	{
		Q_OBJECT
		Q_INTERFACES (IEditorWidget IAdvancedPlainTextEditor IWkFontsSettable)

		QTextEdit * const Edit_;
	public:
		explicit TextEditorAdaptor (QTextEdit*);

		QString GetContents (ContentType type) const override;
		void SetContents (QString contents, ContentType type) override;

		QAction* GetEditorAction (EditorAction) override;
		void AppendAction (QAction*) override;
		void AppendSeparator () override;
		void RemoveAction (QAction*) override;
		void SetBackgroundColor (const QColor&, ContentType) override;
		QWidget* GetWidget () override;
		QObject* GetQObject () override;

		bool FindText (const QString&) override;
		void DeleteSelection () override;

		void SetFontFamily (FontFamily family, const QFont& font) override;
		void SetFontSize (FontSize type, int size) override;
	signals:
		void textChanged () override;
	};
}
}
