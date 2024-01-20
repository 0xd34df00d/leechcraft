/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "fields.h"
#include <algorithm>
#include <QtDebug>
#include "qt5compat.h"
#include <poppler-form.h>
#include <util/sll/unreachable.h>
#include "links.h"

namespace LC
{
namespace Monocle
{
namespace PDF
{
	FormField::FormField (std::shared_ptr<Poppler::FormField> field)
	: BaseField_ (field)
	{
	}

	int FormField::GetID () const
	{
		return BaseField_->id ();
	}

	QRectF FormField::GetRect () const
	{
		return BaseField_->rect ();
	}

	QString FormField::GetName () const
	{
		return BaseField_->uiName ();
	}

	FormFieldText::FormFieldText (std::shared_ptr<Poppler::FormField> field)
	: FormField (field)
	, Field_ (std::dynamic_pointer_cast<Poppler::FormFieldText> (field))
	{
	}

	FormType FormFieldText::GetType () const
	{
		return FormType::Text;
	}

	Qt::Alignment FormFieldText::GetAlignment () const
	{
		return Field_->textAlignment ();
	}

	QString FormFieldText::GetText () const
	{
		return Field_->text ();
	}

	void FormFieldText::SetText (const QString& text)
	{
		Field_->setText (text);
	}

	auto FormFieldText::GetTextType () const -> Type
	{
		switch (Field_->textType ())
		{
		case Poppler::FormFieldText::Normal:
			return Type::SingleLine;
		case Poppler::FormFieldText::Multiline:
			return Type::Multiline;
		case Poppler::FormFieldText::FileSelect:
			return Type::File;
		}

		Util::Unreachable ();
	}

	int FormFieldText::GetMaximumLength () const
	{
		return Field_->maximumLength ();
	}

	bool FormFieldText::IsPassword () const
	{
		return Field_->isPassword ();
	}

	bool FormFieldText::IsRichText () const
	{
		return Field_->isRichText ();
	}

	FormFieldChoice::FormFieldChoice (std::shared_ptr<Poppler::FormField> field)
	: FormField (field)
	, Field_ (std::dynamic_pointer_cast<Poppler::FormFieldChoice> (field))
	{
	}

	FormType FormFieldChoice::GetType () const
	{
		return FormType::Choice;
	}

	Qt::Alignment FormFieldChoice::GetAlignment () const
	{
		return Field_->textAlignment ();
	}

	auto FormFieldChoice::GetChoiceType () const -> Type
	{
		switch (Field_->choiceType ())
		{
		case Poppler::FormFieldChoice::ComboBox:
			return Type::Combobox;
		case Poppler::FormFieldChoice::ListBox:
			return Type::ListBox;
		}

		qWarning () << Q_FUNC_INFO
				<< "unknown choice type"
				<< Field_->choiceType ();
		return Type::Combobox;
	}

	QStringList FormFieldChoice::GetAllChoices () const
	{
		return Field_->choices ();
	}

	QList<int> FormFieldChoice::GetCurrentChoices () const
	{
		return Field_->currentChoices ();
	}

	void FormFieldChoice::SetCurrentChoices (const QList<int>& choices)
	{
		Field_->setCurrentChoices (choices);
	}

	QString FormFieldChoice::GetEditChoice () const
	{
		return Field_->editChoice ();
	}

	void FormFieldChoice::SetEditChoice (const QString& choice)
	{
		Field_->setEditChoice (choice);
	}

	bool FormFieldChoice::IsEditable () const
	{
		return Field_->isEditable ();
	}

	FormFieldButton::FormFieldButton (std::shared_ptr<Poppler::FormField> field, Document *doc)
	: FormField (field)
	, Field_ (std::dynamic_pointer_cast<Poppler::FormFieldButton> (field))
	, Doc_ (doc)
	, ButtonGroup_ (Field_->siblings ())
	{
		if (!ButtonGroup_.isEmpty ())
		{
			ButtonGroup_ << GetID ();
			std::sort (ButtonGroup_.begin (), ButtonGroup_.end ());
		}
	}

	FormType FormFieldButton::GetType () const
	{
		return FormType::Button;
	}

	Qt::Alignment FormFieldButton::GetAlignment () const
	{
		return Qt::AlignLeft;
	}

	auto FormFieldButton::GetButtonType () const -> Type
	{
		switch (Field_->buttonType ())
		{
		case Poppler::FormFieldButton::Push:
			return Type::Pushbutton;
		case Poppler::FormFieldButton::CheckBox:
			return Type::Checkbox;
		case Poppler::FormFieldButton::Radio:
			return Type::Radiobutton;
		}

		qWarning () << Q_FUNC_INFO
				<< "unknown button type"
				<< Field_->buttonType ();
		return Type::Pushbutton;
	}

	QString FormFieldButton::GetCaption () const
	{
		if (!Field_->caption ().isEmpty ())
			return Field_->caption ();
		if (!Field_->uiName ().isEmpty ())
			return Field_->uiName ();
		return Field_->name ();
	}

	bool FormFieldButton::IsChecked () const
	{
		return Field_->state ();
	}

	void FormFieldButton::SetChecked (bool checked)
	{
		Field_->setState (checked);
	}

	QList<int> FormFieldButton::GetButtonGroup () const
	{
		return ButtonGroup_;
	}

	LinkAction FormFieldButton::GetActivationAction () const
	{
		const auto actLink = Field_->activationAction ();
		return actLink ? MakeLinkAction (Doc_, actLink) : NoAction {};
	}
}
}
}
