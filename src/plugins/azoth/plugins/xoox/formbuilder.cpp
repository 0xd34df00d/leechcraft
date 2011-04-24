/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#include "formbuilder.h"
#include <QWidget>
#include <QLabel>
#include <QFormLayout>
#include <QCheckBox>
#include <QTextEdit>
#include <QLineEdit>
#include <QTreeWidget>
#include <QtDebug>

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	class FieldHandler
	{
		QMap<QWidget*, QXmppDataForm::Field*> Widget2Field_;
	public:
		virtual ~FieldHandler () {}
		
		void CreateWidget (QXmppDataForm::Field& field, QFormLayout *layout)
		{
			QWidget *w = CreateWidgetImpl (field, layout);
			if (!w)
				return;
			
			qDebug () << "field" << field.type ()
					<< field.value () << field.options ();

			w->setToolTip (field.description ());
			w->setObjectName (field.key ());
			
			Widget2Field_ [w] = &field;
		}
		
		void Save ()
		{
			Q_FOREACH (QWidget *widget, Widget2Field_.keys ())
			{
				const QVariant& var = GetData (widget);
				if (var.isNull ())
					continue;
				Widget2Field_ [widget]->setValue (var);
			}
		}
	protected:
		virtual QWidget* CreateWidgetImpl (QXmppDataForm::Field&, QFormLayout*) = 0;
		virtual QVariant GetData (QWidget*) = 0;
	};
	
	class BooleanHandler : public FieldHandler
	{
	protected:
		QWidget* CreateWidgetImpl (QXmppDataForm::Field& field, QFormLayout *layout)
		{
			QCheckBox *box = new QCheckBox (field.label ());
			box->setChecked (field.value ().toBool ());
			layout->addWidget (box);
			return box;
		}
		
		QVariant GetData (QWidget *widget)
		{
			QCheckBox *box = qobject_cast<QCheckBox*> (widget);
			if (!box)
			{
				qWarning () << Q_FUNC_INFO
						<< "unable to cast"
						<< widget
						<< "to QCheckBox";
				return QVariant ();
			}
			
			return box->isChecked ();
		}
	};
	
	class FixedHandler : public FieldHandler
	{
	protected:
		QWidget* CreateWidgetImpl (QXmppDataForm::Field& field, QFormLayout *layout)
		{
			QLabel *label = new QLabel (field.value ().toString ());
			layout->addRow (field.label (), label);
			return label;
		}
		
		QVariant GetData (QWidget*)
		{
			return QVariant ();
		}
	};
	
	class NullHandler : public FieldHandler
	{
	protected:
		QWidget* CreateWidgetImpl (QXmppDataForm::Field&, QFormLayout*)
		{
			return 0;
		}
		
		QVariant GetData (QWidget*)
		{
			return QVariant ();
		}
	};
	
	class MultiTextHandler : public FieldHandler
	{
	protected:
		QWidget* CreateWidgetImpl (QXmppDataForm::Field& field, QFormLayout *layout)
		{
			QTextEdit *edit = new QTextEdit (field.value ().toStringList ().join ("\n"));
			layout->addRow (field.label (), edit);
			return edit;
		}
		
		QVariant GetData (QWidget *widget)
		{
			QTextEdit *edit = qobject_cast<QTextEdit*> (widget);
			if (!edit)
			{
				qWarning () << Q_FUNC_INFO
						<< "unable to cast"
						<< widget
						<< "to QTextEdit";
				return QVariant ();
			}
			
			QStringList result = edit->toPlainText ().split ('\n', QString::SkipEmptyParts);
			return result;
		}
	};
	
	class SingleTextHandler : public FieldHandler
	{
		bool IsPassword_;
	public:
		SingleTextHandler (bool pass)
		: IsPassword_ (pass)
		{
		}
	protected:
		QWidget* CreateWidgetImpl (QXmppDataForm::Field& field, QFormLayout *layout)
		{
			QLineEdit *edit = new QLineEdit (field.value ().toString ());
			if (IsPassword_)
				edit->setEchoMode (QLineEdit::Password);
			layout->addRow (field.label (), edit);
			return edit;
		}
		
		QVariant GetData (QWidget *widget)
		{
			QLineEdit *edit = qobject_cast<QLineEdit*> (widget);
			if (!edit)
			{
				qWarning () << Q_FUNC_INFO
						<< "unable to cast"
						<< widget
						<< "to QLineEdit";
				return QVariant ();
			}
			
			return edit->text ();
		}
	};
	
	class ListHandler : public FieldHandler
	{
		QAbstractItemView::SelectionMode SelMode_;
	public:
		ListHandler (QAbstractItemView::SelectionMode mode)
		: SelMode_ (mode)
		{
		}
	protected:
		QWidget* CreateWidgetImpl (QXmppDataForm::Field& field, QFormLayout *layout)
		{
			QTreeWidget *tree = new QTreeWidget ();
			tree->setSelectionMode (SelMode_);
			tree->setHeaderHidden (true);
			
			QPair<QString, QString> option;
			Q_FOREACH (option, field.options ())
			{
				QTreeWidgetItem *item = new QTreeWidgetItem (tree, QStringList (option.first));
				item->setData (0, Qt::UserRole, option.second);
				
				if (option.second == field.value ())
					tree->setCurrentItem (item, 0, QItemSelectionModel::SelectCurrent);
			}
			
			layout->addRow (field.label (), tree);
			return tree;
		}
		
		QVariant GetData (QWidget *widget)
		{
			QTreeWidget *tree = qobject_cast<QTreeWidget*> (widget);
			if (!tree)
			{
				qWarning () << Q_FUNC_INFO
						<< "unable to cast"
						<< widget
						<< "to QTreeWidget";
				return QVariant ();
			}
			
			return tree->currentItem ()->data (0, Qt::UserRole);
		}
	};

	FormBuilder::FormBuilder ()
	{
		Type2Handler_ [QXmppDataForm::Field::BooleanField].reset (new BooleanHandler);
		Type2Handler_ [QXmppDataForm::Field::FixedField].reset (new FixedHandler);
		Type2Handler_ [QXmppDataForm::Field::HiddenField].reset (new NullHandler);
		Type2Handler_ [QXmppDataForm::Field::JidMultiField].reset (new MultiTextHandler);
		Type2Handler_ [QXmppDataForm::Field::JidSingleField].reset (new SingleTextHandler (false));
		Type2Handler_ [QXmppDataForm::Field::ListMultiField].reset (new ListHandler (QAbstractItemView::ExtendedSelection));
		Type2Handler_ [QXmppDataForm::Field::ListSingleField].reset (new ListHandler (QAbstractItemView::SingleSelection));
		Type2Handler_ [QXmppDataForm::Field::TextMultiField].reset (new MultiTextHandler);
		Type2Handler_ [QXmppDataForm::Field::TextPrivateField].reset (new SingleTextHandler (true));
		Type2Handler_ [QXmppDataForm::Field::TextSingleField].reset (new SingleTextHandler (false));
	}
	
	QWidget* FormBuilder::CreateForm (const QXmppDataForm& form, QWidget *parent)
	{
		if (form.isNull ())
			return 0;
		
		Form_ = form;

		QWidget *widget = new QWidget (parent);
		widget->setWindowTitle (form.title ());

		QFormLayout *layout = new QFormLayout;
		widget->setLayout (layout);

		if (!form.title ().isEmpty ())
			layout->addRow (new QLabel (form.title ()));
		if (!form.instructions ().isEmpty ())
			layout->addRow (new QLabel (form.instructions ()));
		
		try
		{
			QList<QXmppDataForm::Field>& fields = Form_.fields ();
			for (int i = 0; i < fields.size (); ++i)
			{
				QXmppDataForm::Field& field = fields [i];
				if (!Type2Handler_.contains (field.type ()))
				{
					qWarning () << Q_FUNC_INFO
							<< "unknown field type"
							<< field.type ();
					continue;
				}
				Type2Handler_ [field.type ()]->CreateWidget (field, layout);
			}
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
					<< e.what ()
					<< "while trying to process fields";
			return 0;
		}

		return widget;
	}
	
	QXmppDataForm FormBuilder::GetForm ()
	{
		Q_FOREACH (FieldHandler_ptr handler, Type2Handler_.values ())
			handler->Save ();
		return Form_;
	}
}
}
}
