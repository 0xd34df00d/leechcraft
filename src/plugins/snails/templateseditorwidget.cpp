/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "templateseditorwidget.h"
#include <QAction>
#include <QMessageBox>
#include <util/sll/either.h>
#include <util/sll/visitor.h>
#include <util/sll/qtutil.h>
#include <interfaces/itexteditor.h>
#include "msgtemplatesmanager.h"
#include "structures.h"

namespace LC
{
namespace Snails
{
	TemplatesEditorWidget::TemplatesEditorWidget (MsgTemplatesManager *mgr, QWidget *parent)
	: QWidget { parent }
	, TemplatesMgr_ { mgr }
	{
		Ui_.setupUi (this);

		connect (Ui_.MessageType_,
				SIGNAL (currentIndexChanged (int)),
				this,
				SLOT (loadTemplate ()));

		connect (Ui_.ContentType_,
				SIGNAL (currentIndexChanged (int)),
				this,
				SLOT (prepareEditor (int)));

		Ui_.Editor_->SetupEditors ([this] (QAction *action)
				{
					EditorTypeActions_ << action;
					Ui_.ContentType_->addItem (action->icon (), action->text ());
				});

		prepareEditor (Ui_.ContentType_->currentIndex ());
	}

	void TemplatesEditorWidget::SaveCurrentText ()
	{
		if (!IsDirty_)
			return;

		const auto currentType = Ui_.Editor_->GetCurrentEditorType ();
		const auto msgType = static_cast<MsgType> (Ui_.MessageType_->currentIndex ());

		const auto& tpl = Ui_.Editor_->GetCurrentEditor ()->GetContents (currentType);

		Unsaved_ [currentType] [msgType] = tpl;

		IsDirty_ = false;
	}

	void TemplatesEditorWidget::accept ()
	{
		SaveCurrentText ();

		const auto localCopy = Unsaved_;
		for (const auto& rootPair : Util::Stlize (localCopy))
		{
			const auto contentType = rootPair.first;

			for (const auto& pair : Util::Stlize (rootPair.second))
			{
				Util::Visit (TemplatesMgr_->SaveTemplate (contentType, pair.first, nullptr, pair.second).AsVariant (),
						[=] (Util::Void)
						{
							auto& submap = Unsaved_ [contentType];
							submap.remove (pair.first);
							if (submap.isEmpty ())
								Unsaved_.remove (contentType);
						},
						[=] (const auto& err)
						{
							QMessageBox::critical (this,
									"LeechCraft",
									tr ("Unable to save template: %1.")
										.arg (err.what ()));
						});
			}
		}
	}

	void TemplatesEditorWidget::reject ()
	{
		IsDirty_ = false;

		loadTemplate ();
	}

	void TemplatesEditorWidget::prepareEditor (int index)
	{
		SaveCurrentText ();

		EditorTypeActions_.value (index)->trigger ();

		loadTemplate ();
	}

	void TemplatesEditorWidget::loadTemplate ()
	{
		const auto currentType = Ui_.Editor_->GetCurrentEditorType ();
		const auto msgType = static_cast<MsgType> (Ui_.MessageType_->currentIndex ());

		Util::Visit (TemplatesMgr_->GetTemplate (currentType, msgType, nullptr).AsVariant (),
				[=] (const QString& tpl)
				{
					auto editor = Ui_.Editor_->GetCurrentEditor ();

					disconnect (editor->GetQObject (),
							SIGNAL (textChanged ()),
							this,
							SLOT (markAsDirty ()));

					editor->SetContents (tpl, currentType);

					connect (editor->GetQObject (),
							SIGNAL (textChanged ()),
							this,
							SLOT (markAsDirty ()));
				},
				[=] (const auto& err)
				{
					QMessageBox::critical (this,
							"LeechCraft",
							tr ("Unable to load template: %1.")
								.arg (err.what ()));
				});
	}

	void TemplatesEditorWidget::markAsDirty ()
	{
		IsDirty_ = true;
	}
}
}
