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
#include <QAbstractItemView>
#include <QTreeWidget>
#include <QCheckBox>
#include <QPushButton>
#include <QRadioButton>
#include <QButtonGroup>
#include <QtDebug>
#include "interfaces/monocle/isupportforms.h"
#include "interfaces/monocle/iformfield.h"
#include "pagegraphicsitem.h"

namespace LC
{
namespace Monocle
{
	FormManager::FormManager (QGraphicsView *view, QObject *parent)
	: QObject (parent)
	, View_ (view)
	, Scene_ (view->scene ())
	{
	}

	void FormManager::HandleDoc (IDocument_ptr doc, const QList<PageGraphicsItem*>& pages)
	{
		Line2Field_.clear ();
		Multiline2Field_.clear ();
		Combo2Field_.clear ();
		List2Field_.clear ();
		Check2Field_.clear ();
		Radio2Field_.clear ();
		Button2Field_.clear ();

		qDeleteAll (RadioGroups_.values ());
		RadioGroups_.clear ();

		auto formsDoc = dynamic_cast<ISupportForms*> (doc.get ());
		if (!formsDoc)
			return;

		for (auto page : pages)
			for (auto field : formsDoc->GetFormFields (page->GetPageNum ()))
			{
				QGraphicsProxyWidget *proxy = 0;
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
						[proxy] (const QRectF& rect)
						{
							proxy->setGeometry (rect);
							proxy->setMinimumSize (rect.size ());
							proxy->setMaximumSize (rect.size ());
						});
			}
	}

	QGraphicsProxyWidget* FormManager::AddTextField (std::shared_ptr<IFormField> baseField)
	{
		const auto field = std::dynamic_pointer_cast<IFormFieldText> (baseField);
		switch (field->GetTextType ())
		{
		case IFormFieldText::Type::SingleLine:
		{
			auto edit = new QLineEdit ();
			edit->setText (field->GetText ());
			if (field->IsPassword ())
				edit->setEchoMode (QLineEdit::Password);
			if (field->GetMaximumLength () > 0)
				edit->setMaxLength (field->GetMaximumLength ());
			edit->setAlignment (baseField->GetAlignment ());

			Line2Field_ [edit] = field;
			connect (edit,
					SIGNAL (textChanged (QString)),
					this,
					SLOT (handleLineEditChanged (QString)));

			return Scene_->addWidget (edit);
		}
		case IFormFieldText::Type::Multiline:
		{
			auto edit = new QTextEdit ();
			edit->setText (field->GetText ());
			edit->setAcceptRichText (field->IsRichText ());
			edit->setAlignment (baseField->GetAlignment ());

			Multiline2Field_ [edit] = field;
			connect (edit,
					SIGNAL (textChanged ()),
					this,
					SLOT (handleTextEditChanged ()));

			return Scene_->addWidget (edit);
		}
		case IFormFieldText::Type::File:
			qWarning () << Q_FUNC_INFO
					<< "unsupported File field type, please send the file to upstream";
			return 0;
		}

		qWarning () << Q_FUNC_INFO
				<< "unsupported type";

		return 0;
	}

	namespace
	{
		class PopupZOrderFixer : public QObject
		{
			QGraphicsProxyWidget *Item_;
			double PrevOrder_;
		public:
			PopupZOrderFixer (QGraphicsProxyWidget *item)
			: QObject (item)
			, Item_ (item)
			, PrevOrder_ (item->zValue ())
			{
			}

			bool eventFilter (QObject*, QEvent *event)
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

	QGraphicsProxyWidget* FormManager::AddChoiceField (std::shared_ptr<IFormField> baseField)
	{
		const auto field = std::dynamic_pointer_cast<IFormFieldChoice> (baseField);
		switch (field->GetChoiceType ())
		{
		case IFormFieldChoice::Type::Combobox:
		{
			auto edit = new QComboBox ();
			edit->setSizeAdjustPolicy (QComboBox::AdjustToContentsOnFirstShow);
			edit->addItems (field->GetAllChoices ());
			edit->setEditable (field->IsEditable ());

			if (field->IsEditable () && !field->GetEditChoice ().isEmpty ())
				edit->setEditText (field->GetEditChoice ());
			else if (!field->GetCurrentChoices ().isEmpty ())
				edit->setCurrentIndex (field->GetCurrentChoices ().first ());

			Combo2Field_ [edit] = field;
			connect (edit,
					SIGNAL (currentIndexChanged (int)),
					this,
					SLOT (handleComboChanged ()));
			connect (edit,
					SIGNAL (editTextChanged (QString)),
					this,
					SLOT (handleComboChanged ()));

			auto proxy = Scene_->addWidget (edit);
			edit->view ()->installEventFilter (new PopupZOrderFixer (proxy));
			return proxy;
		}
		case IFormFieldChoice::Type::ListBox:
			auto edit = new QTreeWidget ();
			for (const auto& choice : field->GetAllChoices ())
				edit->addTopLevelItem (new QTreeWidgetItem ({ choice }));

			const auto& current = field->GetCurrentChoices ();
			for (int i = 0; i < edit->topLevelItemCount (); ++i)
				edit->topLevelItem (i)->setCheckState (0, current.contains (i) ? Qt::Checked : Qt::Unchecked);

			List2Field_ [edit] = field;
			connect (edit,
					SIGNAL (itemChanged (QTreeWidgetItem*, int)),
					this,
					SLOT (handleListChanged ()));

			return Scene_->addWidget (edit);
		}

		qWarning () << Q_FUNC_INFO
				<< "unsupported type";

		return 0;
	}

	QGraphicsProxyWidget* FormManager::AddButtonField (std::shared_ptr<IFormField> baseField)
	{
		const auto field = std::dynamic_pointer_cast<IFormFieldButton> (baseField);
		switch (field->GetButtonType ())
		{
		case IFormFieldButton::Type::Pushbutton:
		{
			auto button = new QPushButton ();
			button->setText (field->GetCaption ());

			Button2Field_ [button] = field;
			connect (button,
					SIGNAL (released ()),
					this,
					SLOT (handleButtonReleased ()));

			return Scene_->addWidget (button);
		}
		case IFormFieldButton::Type::Checkbox:
		{
			auto box = new QCheckBox ();
			box->setText (field->GetCaption ());
			box->setCheckState (field->IsChecked () ? Qt::Checked : Qt::Unchecked);

			Check2Field_ [box] = field;
			connect (box,
					SIGNAL (stateChanged (int)),
					this,
					SLOT (handleCheckboxChanged ()));

			return Scene_->addWidget (box);
		}
		case IFormFieldButton::Type::Radiobutton:
		{
			auto radio = new QRadioButton ();
			radio->setText (field->GetCaption ());
			radio->setChecked (field->IsChecked ());

			const auto& groupID = field->GetButtonGroup ();
			if (!groupID.isEmpty ())
			{
				if (!RadioGroups_.contains (groupID))
					RadioGroups_ [groupID] = new QButtonGroup;
				RadioGroups_ [groupID]->addButton (radio);
			}

			Radio2Field_ [radio] = field;
			connect (radio,
					SIGNAL (toggled (bool)),
					this,
					SLOT (handleRadioChanged ()));

			return Scene_->addWidget (radio);
		}
		}

		qWarning () << Q_FUNC_INFO
				<< "unsupported type";

		return 0;
	}

	void FormManager::handleLineEditChanged (const QString& text)
	{
		Line2Field_ [static_cast<QLineEdit*> (sender ())]->SetText (text);
	}

	void FormManager::handleTextEditChanged ()
	{
		auto edit = qobject_cast<QTextEdit*> (sender ());
		Multiline2Field_ [edit]->SetText (edit->toPlainText ());
	}

	void FormManager::handleComboChanged ()
	{
		auto box = qobject_cast<QComboBox*> (sender ());
		auto field = Combo2Field_ [box];

		const auto& text = box->currentText ();
		field->SetEditChoice (text);

		if (box->currentIndex () >= 0)
			field->SetCurrentChoices ({ box->currentIndex () });
	}

	void FormManager::handleListChanged ()
	{
		auto edit = qobject_cast<QTreeWidget*> (sender ());

		QList<int> choices;
		for (int i = 0; i < edit->topLevelItemCount (); ++i)
			if (edit->topLevelItem (i)->checkState (0) == Qt::Checked)
				choices << i;

		List2Field_ [edit]->SetCurrentChoices (choices);
	}

	void FormManager::handleCheckboxChanged ()
	{
		auto box = qobject_cast<QCheckBox*> (sender ());
		Check2Field_ [box]->SetChecked (box->checkState () == Qt::Checked);
	}

	void FormManager::handleRadioChanged ()
	{
		auto radio = qobject_cast<QRadioButton*> (sender ());
		Radio2Field_ [radio]->SetChecked (radio->isChecked ());
	}

	void FormManager::handleButtonReleased ()
	{
		auto button = qobject_cast<QPushButton*> (sender ());
		Button2Field_ [button]->HandleActivated ();
	}
}
}
