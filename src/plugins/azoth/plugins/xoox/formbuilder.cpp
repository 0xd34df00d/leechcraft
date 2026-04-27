/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "formbuilder.h"
#include <QWidget>
#include <QLabel>
#include <QFormLayout>
#include <QCheckBox>
#include <QTextEdit>
#include <QLineEdit>
#include <QTreeWidget>
#include <QMimeType>
#include <QtDebug>
#include <util/sll/qtutil.h>
#include <util/sll/prelude.h>
#include "imagemediawidget.h"
#include "xeps/xmppbobmanager.h"

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	class FieldHandler
	{
		QMap<QWidget*, QXmppDataForm::Field> Widget2Field_;
	public:
		virtual ~FieldHandler () = default;

		void CreateWidget (const QXmppDataForm::Field& field, QFormLayout *layout)
		{
			QWidget *w = CreateWidgetImpl (field, layout);
			if (!w)
				return;

			qDebug () << "field" << field.type ()
					<< field.value () << field.options ();

			w->setToolTip (field.description ());
			w->setObjectName (field.key ());

			w->setProperty ("Azoth/Xoox/IsRequired", field.isRequired ());

			Widget2Field_ [w] = field;
		}

		QHash<QString, QXmppDataForm::Field> GetFields ()
		{
			QHash<QString, QXmppDataForm::Field> result;
			for (const auto& [widget, field] : Widget2Field_.asKeyValueRange ())
				if (const auto& var = GetData (widget);
					!var.isNull ())
				{
					auto& updatedField = result [field.key ()];
					updatedField = field;
					updatedField.setValue (var);
				}
			return result;
		}
	protected:
		virtual QWidget* CreateWidgetImpl (const QXmppDataForm::Field&, QFormLayout*) = 0;
		virtual QVariant GetData (QWidget*) = 0;
	};

	namespace
	{
		QWidget* CombineWithMedia (const QVector<QXmppDataForm::MediaSource>& medias, QWidget *widget, FormBuilder *builder)
		{
			auto container = new QWidget;
			auto layout = new QVBoxLayout ();
			QWidget *mediaWidget = nullptr;

			const auto& media = medias.first ();

			if (media.contentType ().name ().startsWith ("image/"))
				mediaWidget = new ImageMediaWidget (media.uri (), builder->BobManager (), builder->From (), container);

			if (!mediaWidget)
			{
				mediaWidget = new QLabel (QObject::tr ("Unable to represent embedded media data."));
				qWarning () << Q_FUNC_INFO
						<< "unable to process "
						<< media.uri ();
			}
			layout->addWidget (mediaWidget);
			layout->addWidget (widget);
			container->setLayout (layout);
			return container;
		}
	}

	template<typename WidgetT>
	class TypedFieldHandler : public FieldHandler
	{
	protected:
		virtual QVariant GetDataImpl (WidgetT*) = 0;

		QVariant GetData (QWidget *widget) final
		{
			if (const auto concrete = qobject_cast<WidgetT*> (widget))
				return GetDataImpl (concrete);
			qWarning () << "unable to cast" << widget;
			throw std::runtime_error { "unable to cast" + widget->objectName ().toStdString () };
		}

		WidgetT* CreateWidgetImpl (const QXmppDataForm::Field&, QFormLayout*) override = 0;
	};

	class BooleanHandler : public TypedFieldHandler<QCheckBox>
	{
	protected:
		QCheckBox* CreateWidgetImpl (const QXmppDataForm::Field& field, QFormLayout *layout) override
		{
			auto box = new QCheckBox (field.label ());
			box->setChecked (field.value ().toBool ());
			layout->addWidget (box);
			return box;
		}

		QVariant GetDataImpl (QCheckBox *box) final
		{
			return box->isChecked ();
		}
	};

	class FixedHandler : public FieldHandler
	{
	protected:
		QWidget* CreateWidgetImpl (const QXmppDataForm::Field& field, QFormLayout *layout) override
		{
			auto label = new QLabel (field.value ().toString ());
			layout->addRow (field.label (), label);
			return label;
		}

		QVariant GetData (QWidget*) override
		{
			return {};
		}
	};

	class NullHandler : public FieldHandler
	{
	protected:
		QWidget* CreateWidgetImpl (const QXmppDataForm::Field&, QFormLayout*) override
		{
			return nullptr;
		}

		QVariant GetData (QWidget*) override
		{
			return {};
		}
	};

	class MultiTextHandler : public TypedFieldHandler<QTextEdit>
	{
	protected:
		QTextEdit* CreateWidgetImpl (const QXmppDataForm::Field& field, QFormLayout *layout) override
		{
			auto edit = new QTextEdit;
			edit->setAcceptRichText (false);
			edit->setText (field.value ().toStringList ().join ("\n"));
			layout->addRow (field.label (), edit);
			return edit;
		}

		QVariant GetDataImpl (QTextEdit *edit) final
		{
			return edit->toPlainText ().split ('\n', Qt::SkipEmptyParts);
		}
	};

	class SingleTextHandler : public TypedFieldHandler<QLineEdit>
	{
		FormBuilder *Builder_;
		bool IsPassword_;
	public:
		SingleTextHandler (bool pass, FormBuilder *builder)
		: Builder_ (builder)
		, IsPassword_ (pass)
		{
		}
	protected:
		QLineEdit* CreateWidgetImpl (const QXmppDataForm::Field& field, QFormLayout *layout) override
		{
			auto edit = new QLineEdit (field.value ().toString ());
			if (IsPassword_)
				edit->setEchoMode (QLineEdit::Password);
			if (!field.mediaSources ().isEmpty ())
				layout->addRow (field.label (), CombineWithMedia (field.mediaSources (), edit, Builder_));
			else
				layout->addRow (field.label (), edit);
			return edit;
		}

		QVariant GetDataImpl (QLineEdit *edit) final
		{
			return edit->text ();
		}
	};

	class ListHandler : public TypedFieldHandler<QTreeWidget>
	{
		QAbstractItemView::SelectionMode SelMode_;
	public:
		explicit ListHandler (QAbstractItemView::SelectionMode mode)
		: SelMode_ (mode)
		{
		}
	protected:
		QTreeWidget* CreateWidgetImpl (const QXmppDataForm::Field& field, QFormLayout *layout) override
		{
			auto tree = new QTreeWidget ();
			tree->setSelectionMode (SelMode_);
			tree->setHeaderHidden (true);

			const auto& selectedValues = field.value ().toStringList ();

			for (const auto& [name, value] : field.options ())
			{
				auto item = new QTreeWidgetItem (tree, { name });
				item->setData (0, Qt::UserRole, value);
				if (selectedValues.contains (value))
					item->setSelected (true);
			}

			layout->addRow (field.label (), tree);
			return tree;
		}

		QVariant GetDataImpl (QTreeWidget *tree) final
		{
			return Util::Map (tree->selectedItems (),
					[] (auto item) { return item->data (0, Qt::UserRole).toString (); });
		}
	};

	FormBuilder::FormBuilder (const QXmppDataForm& form, const QString& from, XMPPBobManager *bobManager)
	: Form_ { form }
	, From_ { from }
	, BobManager_ { bobManager }
	{
		Type2Handler_ [QXmppDataForm::Field::BooleanField].reset (new BooleanHandler ());
		Type2Handler_ [QXmppDataForm::Field::FixedField].reset (new FixedHandler ());
		Type2Handler_ [QXmppDataForm::Field::HiddenField].reset (new NullHandler ());
		Type2Handler_ [QXmppDataForm::Field::JidMultiField].reset (new MultiTextHandler ());
		Type2Handler_ [QXmppDataForm::Field::JidSingleField].reset (new SingleTextHandler (false, this));
		Type2Handler_ [QXmppDataForm::Field::ListMultiField].reset (new ListHandler (QAbstractItemView::ExtendedSelection));
		Type2Handler_ [QXmppDataForm::Field::ListSingleField].reset (new ListHandler (QAbstractItemView::SingleSelection));
		Type2Handler_ [QXmppDataForm::Field::TextMultiField].reset (new MultiTextHandler ());
		Type2Handler_ [QXmppDataForm::Field::TextPrivateField].reset (new SingleTextHandler (true, this));
		Type2Handler_ [QXmppDataForm::Field::TextSingleField].reset (new SingleTextHandler (false, this));
	}

	XMPPBobManager* FormBuilder::BobManager () const
	{
		return BobManager_;
	}

	QString FormBuilder::From () const
	{
		return From_;
	}

	QWidget* FormBuilder::CreateForm (QWidget *parent)
	{
		if (Form_.isNull ())
			return nullptr;

		auto widget = new QWidget (parent);
		widget->setWindowTitle (Form_.title ());

		auto layout = new QFormLayout;
		widget->setLayout (layout);

		if (!Form_.title ().isEmpty ())
			layout->addRow (new QLabel (Form_.title ()));
		if (!Form_.instructions ().isEmpty ())
			layout->addRow (new QLabel (Form_.instructions ()));

		try
		{
			for (const auto& field : Form_.fields ())
				if (const auto handler = Type2Handler_ [field.type ()])
					handler->CreateWidget (field, layout);
				else
					qWarning () << "unknown field type" << field.type ();
		}
		catch (const std::exception& e)
		{
			qWarning () << e.what () << "while trying to process fields";
			return nullptr;
		}

		return widget;
	}

	QXmppDataForm FormBuilder::GetUpdatedForm (QXmppDataForm::Type type) const
	{
		QHash<QString, QXmppDataForm::Field> updatedFields;
		for (const auto& handler : Type2Handler_)
			updatedFields.insert (handler->GetFields ());

		auto fields = Form_.fields ();
		for (auto& field : fields)
			if (const auto it = updatedFields.find (field.key ());
				it != updatedFields.end ())
				field = *it;

		auto result = Form_;
		result.setType (type);
		result.setFields (fields);
		return result;
	}

	namespace
	{
		QString GetFieldVal (const auto& handlers, const QString& name)
		{
			for (const auto& handler : handlers)
			{
				const auto& fields = handler->GetFields ();
				if (const auto it = fields.find (name);
					it != fields.end ())
					return it->value ().toString ();
			}

			return {};
		}
	}

	QString FormBuilder::GetUsername () const
	{
		return GetFieldVal (Type2Handler_, "username");
	}

	QString FormBuilder::GetPassword () const
	{
		return GetFieldVal (Type2Handler_, "password");
	}
}
}
}
