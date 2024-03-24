/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
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

	class FormField : public QObject
					, public IFormField
	{
		Q_OBJECT
		Q_INTERFACES (LC::Monocle::IFormField)

		std::shared_ptr<Poppler::FormField> BaseField_;
	protected:
		FormField (std::shared_ptr<Poppler::FormField>);
	public:
		int GetID () const override;
		QRectF GetRect () const override;
		QString GetName () const override;
	};

	class FormFieldText final : public FormField
							  , public IFormFieldText
	{
		Q_INTERFACES (LC::Monocle::IFormField
				LC::Monocle::IFormFieldText)

		std::shared_ptr<Poppler::FormFieldText> Field_;
	public:
		FormFieldText (std::shared_ptr<Poppler::FormField>);

		FormType GetType () const override;
		Qt::Alignment GetAlignment () const override;

		QString GetText () const override;
		void SetText (const QString&) override;
		Type GetTextType () const override;

		int GetMaximumLength () const override;
		bool IsPassword () const override;
		bool IsRichText () const override;
	};

	class FormFieldChoice final : public FormField
								, public IFormFieldChoice
	{
		Q_INTERFACES (LC::Monocle::IFormField
				LC::Monocle::IFormFieldChoice)

		std::shared_ptr<Poppler::FormFieldChoice> Field_;
	public:
		FormFieldChoice (std::shared_ptr<Poppler::FormField>);

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

	class FormFieldButton final : public FormField
						        , public IFormFieldButton
	{
		Q_INTERFACES (LC::Monocle::IFormField
				LC::Monocle::IFormFieldButton)

		std::shared_ptr<Poppler::FormFieldButton> Field_;
		Document * const Doc_;
		QList<int> ButtonGroup_;
	public:
		FormFieldButton (std::shared_ptr<Poppler::FormField>, Document*);

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
