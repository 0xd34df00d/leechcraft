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
		QMap<QWidget*, QXmppDataForm::Field*> Widget2Field_;
	public:
		virtual ~FieldHandler () = default;

		void CreateWidget (QXmppDataForm::Field& field, QFormLayout *layout)
		{
			QWidget *w = CreateWidgetImpl (field, layout);
			if (!w)
				return;

			qDebug () << "field" << field.type ()
					<< field.value () << field.options ();

			w->setToolTip (field.description ());
			w->setObjectName (field.key ());

			w->setProperty ("Azoth/Xoox/IsRequired", field.isRequired ());

			Widget2Field_ [w] = &field;
		}

		void Save ()
		{
			for (const auto& [widget, field] : Util::Stlize (Widget2Field_))
			{
				if (const auto& var = GetData (widget); !var.isNull ())
					field->setValue (var);
			}
		}

		void Clear ()
		{
			qDeleteAll (Widget2Field_.keys ());
			Widget2Field_.clear ();
		}
	protected:
		virtual QWidget* CreateWidgetImpl (QXmppDataForm::Field&, QFormLayout*) = 0;
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
			auto concrete = qobject_cast<WidgetT*> (widget);
			if (!concrete)
			{
				qWarning () << Q_FUNC_INFO
						<< "unable to cast"
						<< widget;
				return {};
			}
			return GetDataImpl (concrete);
		}
	};

	class BooleanHandler : public TypedFieldHandler<QCheckBox>
	{
	protected:
		QWidget* CreateWidgetImpl (QXmppDataForm::Field& field, QFormLayout *layout) override
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
		QWidget* CreateWidgetImpl (QXmppDataForm::Field& field, QFormLayout *layout) override
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
		QWidget* CreateWidgetImpl (QXmppDataForm::Field&, QFormLayout*) override
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
		QWidget* CreateWidgetImpl (QXmppDataForm::Field& field, QFormLayout *layout) override
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
		QWidget* CreateWidgetImpl (QXmppDataForm::Field& field, QFormLayout *layout) override
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
		QWidget* CreateWidgetImpl (QXmppDataForm::Field& field, QFormLayout *layout) override
		{
			auto tree = new QTreeWidget ();
			tree->setSelectionMode (SelMode_);
			tree->setHeaderHidden (true);

			for (const auto& [name, value] : field.options ())
			{
				auto item = new QTreeWidgetItem (tree, { name });
				item->setData (0, Qt::UserRole, value);

				if (value == field.value ())
					tree->setCurrentItem (item, 0, QItemSelectionModel::SelectCurrent);
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

	FormBuilder::FormBuilder (const QString& from, XMPPBobManager *bobManager)
	: From_ (from)
	, BobManager_ (bobManager)
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

	void FormBuilder::Clear ()
	{
		for (auto& handler : Type2Handler_)
			handler->Clear ();
	}

	XMPPBobManager* FormBuilder::BobManager () const
	{
		return BobManager_;
	}

	QString FormBuilder::From () const
	{
		return From_;
	}

	QWidget* FormBuilder::CreateForm (const QXmppDataForm& form, QWidget *parent)
	{
		if (form.isNull ())
			return nullptr;

		Form_ = form;

		auto widget = new QWidget (parent);
		widget->setWindowTitle (form.title ());

		auto layout = new QFormLayout;
		widget->setLayout (layout);

		if (!form.title ().isEmpty ())
			layout->addRow (new QLabel (form.title ()));
		if (!form.instructions ().isEmpty ())
			layout->addRow (new QLabel (form.instructions ()));

		try
		{
			for (auto& field : Form_.fields ())
			{
				const auto handler = Type2Handler_ [field.type ()];
				if (!handler)
				{
					qWarning () << Q_FUNC_INFO
							<< "unknown field type"
							<< field.type ();
					continue;
				}
				handler->CreateWidget (field, layout);
			}
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
					<< e.what ()
					<< "while trying to process fields";
			return nullptr;
		}

		return widget;
	}

	QXmppDataForm FormBuilder::GetForm ()
	{
		for (const auto& handler : Type2Handler_)
			handler->Save ();
		return Form_;
	}

	namespace
	{
		QString GetFieldVal (const QXmppDataForm& form, const QString& name)
		{
			for (const auto& field : form.fields ())
				if (field.key () == name)
					return field.value ().toString ();

			return {};
		}
	}

	QString FormBuilder::GetSavedUsername () const
	{
		return GetFieldVal (Form_, "username");
	}

	QString FormBuilder::GetSavedPass () const
	{
		return GetFieldVal (Form_, "password");
	}
}
}
}
