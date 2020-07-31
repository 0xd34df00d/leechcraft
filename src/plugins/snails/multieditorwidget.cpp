/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "multieditorwidget.h"
#include <QAction>
#include <QActionGroup>
#include <util/sll/prelude.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/iinfo.h>
#include "core.h"
#include "texteditoradaptor.h"

namespace LC
{
namespace Snails
{
	MultiEditorWidget::MultiEditorWidget (QWidget *parent)
	: QWidget { parent }
	{
		Ui_.setupUi (this);
	}

	void MultiEditorWidget::SetupEditors (const std::function<void (QAction*)>& editorsHandler)
	{
		const auto editorsGroup = new QActionGroup (this);
		auto addEditor = [&] (const QString& name, std::shared_ptr<IEditorWidget> editor)
		{
			const auto index = MsgEdits_.size ();

			MsgEdits_ << std::move (editor);

			auto action = new QAction (name, this);
			connect (action,
					&QAction::triggered,
					this,
					[this, index] { HandleEditorSelected (index); });
			editorsGroup->addAction (action);
			action->setCheckable (true);
			action->setChecked (true);

			editorsHandler (action);
			Actions_ << action;
		};

		addEditor (tr ("Plain text (internal)"), std::make_shared<TextEditorAdaptor> (Ui_.PlainEdit_));

		const auto& plugs = Core::Instance ().GetProxy ()->
				GetPluginsManager ()->GetAllCastableRoots<ITextEditor*> ();
		for (const auto plugObj : plugs)
		{
			const auto plug = qobject_cast<ITextEditor*> (plugObj);

			if (!plug->SupportsEditor (ContentType::HTML))
				continue;

			const std::shared_ptr<QWidget> w { plug->GetTextEditor (ContentType::HTML) };
			auto edit = std::dynamic_pointer_cast<IEditorWidget> (w);
			if (!edit)
				continue;

			Ui_.EditorStack_->addWidget (w.get ());

			const auto& pluginName = qobject_cast<IInfo*> (plugObj)->GetName ();
			addEditor (tr ("Rich text (%1)").arg (pluginName), std::move (edit));
		}

		Ui_.EditorStack_->setCurrentIndex (Ui_.EditorStack_->count () - 1);
	}

	ContentType MultiEditorWidget::GetCurrentEditorType () const
	{
		return Ui_.EditorStack_->currentIndex () ? ContentType::HTML : ContentType::PlainText;
	}

	IEditorWidget* MultiEditorWidget::GetCurrentEditor () const
	{
		return MsgEdits_.value (Ui_.EditorStack_->currentIndex ()).get ();
	}

	void MultiEditorWidget::SelectEditor (ContentType type)
	{
		int index = 0;
		switch (type)
		{
		case ContentType::PlainText:
			index = 0;
			break;
		case ContentType::HTML:
			index = 1;
			break;
		}

		if (index >= Actions_.size ())
			return;

		Actions_.value (index)->setChecked (true);
		HandleEditorSelected (index);
	}

	QList<IEditorWidget*> MultiEditorWidget::GetAllEditors () const
	{
		return Util::Map (MsgEdits_, [] (auto ptr) { return ptr.get (); });
	}

	ContentType MultiEditorWidget::GetEditorType (IEditorWidget *editor) const
	{
		return editor == MsgEdits_.value (0).get () ?
				ContentType::PlainText :
				ContentType::HTML;
	}

	void MultiEditorWidget::HandleEditorSelected (int index)
	{
		const auto previous = GetCurrentEditor ();
		Ui_.EditorStack_->setCurrentIndex (index);
		emit editorChanged (GetCurrentEditor (), previous);
	}
}
}
