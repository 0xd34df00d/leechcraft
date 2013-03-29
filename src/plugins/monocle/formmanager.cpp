/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "formmanager.h"
#include <QGraphicsView>
#include <QGraphicsProxyWidget>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QAbstractItemView>
#include <QTreeWidget>
#include <QtDebug>
#include "interfaces/monocle/isupportforms.h"
#include "interfaces/monocle/iformfield.h"
#include "pagegraphicsitem.h"

namespace LeechCraft
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
				}

				if (!proxy)
					continue;

				const auto& docRect = page->MapToDoc (page->boundingRect ());
				const auto& formRect = field->GetRect ();

				QRectF targetRect (formRect.x () * docRect.width (),
						formRect.y () * docRect.height (),
						formRect.width () * docRect.width (),
						formRect.height () * docRect.height ());

				proxy->setParentItem (page);
				page->RegisterChildRect (proxy, targetRect,
						[proxy] (const QRectF& rect) { proxy->setGeometry (rect); });
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
}
}
