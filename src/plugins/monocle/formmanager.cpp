/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "formmanager.h"
#include <QGraphicsView>
#include <QGraphicsProxyWidget>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QTreeWidget>
#include <QCheckBox>
#include <QPushButton>
#include <QRadioButton>
#include <QButtonGroup>
#include <QtDebug>
#include "interfaces/monocle/isupportforms.h"
#include "interfaces/monocle/iformfield.h"
#include "components/layout/positions.h"
#include "components/services/linkactionexecutor.h"
#include "components/viewitems/pagegraphicsitem.h"

namespace LC::Monocle
{
	FormManager::FormManager (QGraphicsView *view, LinkExecutionContext& ec)
	: QObject { view }
	, ExecutionContext_ { ec }
	, Scene_ { view->scene () }
	{
	}

	void FormManager::HandleDoc (IDocument& doc, const QVector<PageGraphicsItem*>& pages)
	{
		qDeleteAll (RadioGroups_);
		RadioGroups_.clear ();

		auto formsDoc = dynamic_cast<ISupportForms*> (&doc);
		if (!formsDoc)
			return;

		for (auto page : pages)
			for (const auto& field : formsDoc->GetFormFields (page->GetPageNum ()))
			{
				QGraphicsProxyWidget *proxy = nullptr;
				switch (field->GetType ())
				{
				case FormType::Text:
					proxy = AddTextField (field);
					break;
				case FormType::Choice:
					proxy = AddChoiceField (field);
					break;
				case FormType::Button:
					proxy = AddButtonField (field);
					break;
				}

				if (!proxy)
					continue;

				proxy->setParentItem (page);
				page->RegisterChildRect (proxy, field->GetRect (),
						[proxy] (const PageAbsoluteRect& pageRect)
						{
							const auto& rect = pageRect.ToRectF ();
							proxy->setGeometry (rect);
							proxy->setMinimumSize (rect.size ());
							proxy->setMaximumSize (rect.size ());
						});
			}
	}

	QGraphicsProxyWidget* FormManager::AddTextField (const std::shared_ptr<IFormField>& baseField)
	{
		const auto field = std::dynamic_pointer_cast<IFormFieldText> (baseField);
		switch (field->GetTextType ())
		{
		case IFormFieldText::Type::SingleLine:
		{
			auto edit = new QLineEdit;
			edit->setText (field->GetText ());
			if (field->IsPassword ())
				edit->setEchoMode (QLineEdit::Password);
			if (field->GetMaximumLength () > 0)
				edit->setMaxLength (field->GetMaximumLength ());
			edit->setAlignment (baseField->GetAlignment ());

			connect (edit,
					&QLineEdit::textChanged,
					this,
					[field] (const QString& text) { field->SetText (text); });

			return Scene_->addWidget (edit);
		}
		case IFormFieldText::Type::Multiline:
		{
			auto edit = new QTextEdit;
			edit->setText (field->GetText ());
			edit->setAcceptRichText (field->IsRichText ());
			edit->setAlignment (baseField->GetAlignment ());

			connect (edit,
					&QTextEdit::textChanged,
					this,
					[=] { field->SetText (edit->toPlainText ()); });

			return Scene_->addWidget (edit);
		}
		case IFormFieldText::Type::File:
			qWarning () << "unsupported File field type, please send the file to upstream";
			return nullptr;
		}

		qWarning () << "unsupported type" << static_cast<int> (field->GetTextType ());
		return nullptr;
	}

	namespace
	{
		class PopupZOrderFixer : public QObject
		{
			QGraphicsProxyWidget *Item_;
			double PrevOrder_;
		public:
			PopupZOrderFixer (QGraphicsProxyWidget *item)
			: QObject { item }
			, Item_ { item }
			, PrevOrder_ { item->zValue () }
			{
			}

			bool eventFilter (QObject*, QEvent *event) override
			{
				switch (event->type ())
				{
				case QEvent::Show:
					Item_->setZValue (PrevOrder_ + 1);
					break;
				case QEvent::Hide:
					Item_->setZValue (PrevOrder_);
					break;
				default:
					break;
				}

				return false;
			}
		};
	}

	QGraphicsProxyWidget* FormManager::AddChoiceField (const std::shared_ptr<IFormField>& baseField)
	{
		const auto field = std::dynamic_pointer_cast<IFormFieldChoice> (baseField);
		switch (field->GetChoiceType ())
		{
		case IFormFieldChoice::Type::Combobox:
		{
			auto edit = new QComboBox;
			edit->setSizeAdjustPolicy (QComboBox::AdjustToContentsOnFirstShow);
			edit->addItems (field->GetAllChoices ());
			edit->setEditable (field->IsEditable ());

			if (field->IsEditable () && !field->GetEditChoice ().isEmpty ())
				edit->setEditText (field->GetEditChoice ());
			else if (!field->GetCurrentChoices ().isEmpty ())
				edit->setCurrentIndex (field->GetCurrentChoices ().first ());

			auto updateField = [=]
			{
				field->SetEditChoice (edit->currentText ());
				if (const auto idx = edit->currentIndex (); idx >= 0)
					field->SetCurrentChoices ({ idx });
			};
			connect (edit,
					&QComboBox::currentIndexChanged,
					this,
					updateField);
			connect (edit,
					&QComboBox::editTextChanged,
					this,
					updateField);

			auto proxy = Scene_->addWidget (edit);
			edit->view ()->installEventFilter (new PopupZOrderFixer (proxy));
			return proxy;
		}
		case IFormFieldChoice::Type::ListBox:
			auto edit = new QTreeWidget;
			for (const auto& choice : field->GetAllChoices ())
				edit->addTopLevelItem (new QTreeWidgetItem ({ choice }));

			const auto& current = field->GetCurrentChoices ();
			for (int i = 0; i < edit->topLevelItemCount (); ++i)
				edit->topLevelItem (i)->setCheckState (0, current.contains (i) ? Qt::Checked : Qt::Unchecked);

			connect (edit,
					&QTreeWidget::itemChanged,
					this,
					[=]
					{
						QList<int> choices;
						for (int i = 0; i < edit->topLevelItemCount (); ++i)
							if (edit->topLevelItem (i)->checkState (0) == Qt::Checked)
								choices << i;
						field->SetCurrentChoices (choices);
					});

			return Scene_->addWidget (edit);
		}

		qWarning () << "unsupported type" << static_cast<int> (field->GetChoiceType ());
		return 0;
	}

	QGraphicsProxyWidget* FormManager::AddButtonField (const std::shared_ptr<IFormField>& baseField)
	{
		const auto field = std::dynamic_pointer_cast<IFormFieldButton> (baseField);
		switch (field->GetButtonType ())
		{
		case IFormFieldButton::Type::Pushbutton:
		{
			auto button = new QPushButton;
			button->setText (field->GetCaption ());
			connect (button,
					&QPushButton::released,
					this,
					[this, field] { ExecuteLinkAction (field->GetActivationAction (), ExecutionContext_); });

			return Scene_->addWidget (button);
		}
		case IFormFieldButton::Type::Checkbox:
		{
			auto box = new QCheckBox;
			box->setText (field->GetCaption ());
			box->setCheckState (field->IsChecked () ? Qt::Checked : Qt::Unchecked);

			connect (box,
					&QCheckBox::stateChanged,
					this,
					[field] (int state) { field->SetChecked (state == Qt::Checked); });

			return Scene_->addWidget (box);
		}
		case IFormFieldButton::Type::Radiobutton:
		{
			auto radio = new QRadioButton;
			radio->setText (field->GetCaption ());
			radio->setChecked (field->IsChecked ());

			if (const auto& groupID = field->GetButtonGroup ();
				!groupID.isEmpty ())
			{
				if (!RadioGroups_.contains (groupID))
					RadioGroups_ [groupID] = new QButtonGroup;
				RadioGroups_ [groupID]->addButton (radio);
			}

			connect (radio,
					&QRadioButton::toggled,
					this,
					[field] (bool checked) { field->SetChecked (checked); });

			return Scene_->addWidget (radio);
		}
		}

		qWarning () << "unsupported type" << static_cast<int> (field->GetButtonType ());
		return nullptr;
	}
}
