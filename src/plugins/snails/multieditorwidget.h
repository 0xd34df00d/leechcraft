/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <memory>
#include <QWidget>
#include "ui_multieditorwidget.h"

class IEditorWidget;

namespace LC
{
enum class ContentType;

namespace Snails
{
	class MultiEditorWidget : public QWidget
	{
		Q_OBJECT

		Ui::MultiEditorWidget Ui_;

		QList<std::shared_ptr<IEditorWidget>> MsgEdits_;

		QList<QAction*> Actions_;
	public:
		MultiEditorWidget (QWidget* = nullptr);

		void SetupEditors (const std::function<void (QAction*)>&);

		ContentType GetCurrentEditorType () const;
		IEditorWidget* GetCurrentEditor () const;
		void SelectEditor (ContentType);

		QList<IEditorWidget*> GetAllEditors () const;
		ContentType GetEditorType (IEditorWidget*) const;
	private:
		void HandleEditorSelected (int);
	signals:
		void editorChanged (IEditorWidget*, IEditorWidget*);
	};
}
}
