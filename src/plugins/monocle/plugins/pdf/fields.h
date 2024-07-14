/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <interfaces/monocle/iformfield.h>

namespace Poppler
{
	class FormField;
	class FormFieldText;
	class FormFieldChoice;
	class FormFieldButton;
}

namespace LC::Monocle::PDF
{
	class Document;

	template<typename T>
	class FormField : public IFormField
	{
	protected:
		std::shared_ptr<T> Field_;
	public:
		explicit FormField (const std::shared_ptr<Poppler::FormField>& field)
		: Field_ { std::dynamic_pointer_cast<T> (field) }
		{
		}

		int GetID () const override
		{
			return Field_->id ();
		}

		PageRelativeRectBase GetRect () const override
		{
			return PageRelativeRectBase { Field_->rect () };
		}

		QString GetName () const override
		{
			return Field_->uiName ();
		}
	};

	class FormFieldText final : public FormField<Poppler::FormFieldText>
							  , public IFormFieldText
	{
	public:
		using FormField::FormField;

		FormType GetType () const override;
		Qt::Alignment GetAlignment () const override;

		QString GetText () const override;
		void SetText (const QString&) override;
		Type GetTextType () const override;

		int GetMaximumLength () const override;
		bool IsPassword () const override;
		bool IsRichText () const override;
	};

	class FormFieldChoice final : public FormField<Poppler::FormFieldChoice>
								, public IFormFieldChoice
	{
		std::shared_ptr<Poppler::FormFieldChoice> Field_;
	public:
		using FormField::FormField;

		FormType GetType () const override;
		Qt::Alignment GetAlignment () const override;

		Type GetChoiceType () const override;

		QStringList GetAllChoices () const override;

		QList<int> GetCurrentChoices () const override;
		void SetCurrentChoices (const QList<int>&) override;

		QString GetEditChoice () const override;
		void SetEditChoice (const QString&) override;

		bool IsEditable () const override;
	};

	class FormFieldButton final : public FormField<Poppler::FormFieldButton>
						        , public IFormFieldButton
	{
		std::shared_ptr<Poppler::FormFieldButton> Field_;
		Document * const Doc_;
		QList<int> ButtonGroup_;
	public:
		FormFieldButton (const std::shared_ptr<Poppler::FormField>&, Document*);

		FormType GetType () const override;
		Qt::Alignment GetAlignment () const override;

		Type GetButtonType () const override;

		QString GetCaption () const override;

		bool IsChecked () const override;
		void SetChecked (bool) override;

		QList<int> GetButtonGroup () const override;

		LinkAction GetActivationAction () const override;
	};
}
