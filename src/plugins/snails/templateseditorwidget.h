/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
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
		QMap<ContentType, QMap<MsgType, QString>> Unsaved_;
	public:
		TemplatesEditorWidget (MsgTemplatesManager*, QWidget* = nullptr);
	private:
		void SaveCurrentText ();
	public slots:
		void accept ();
		void reject ();
	private slots:
		void prepareEditor (int);
		void loadTemplate ();
		void markAsDirty ();
	};
}
}
