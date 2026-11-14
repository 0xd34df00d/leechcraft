/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include <util/threads/coro/context.h>
#include "ui_templateseditorwidget.h"

namespace LC
{
enum class ContentType;

namespace Snails
{
	class MsgTemplatesManager;

	enum class MsgType;

	class TemplatesEditorWidget : public QWidget
	{
		Q_OBJECT

		Ui::TemplatesEditorWidget Ui_;

		MsgTemplatesManager * const TemplatesMgr_;

		QList<QAction*> EditorTypeActions_;

		bool IsDirty_ = false;
		int DirtyGeneration_ = 0;
		QMap<ContentType, QMap<MsgType, QString>> Unsaved_;
	public:
		TemplatesEditorWidget (MsgTemplatesManager*, QWidget* = nullptr);
	private:
		Util::ContextTask<void> SaveCurrentText ();
	public slots:
		Util::ContextTask<void> accept ();
		void reject ();
	private slots:
		Util::ContextTask<void> prepareEditor (int);
		void loadTemplate ();
		void markAsDirty ();

		LC_CORO_CONTEXT
	};
}
}
