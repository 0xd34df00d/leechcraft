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

namespace LC
{
namespace Monocle
{
namespace PDF
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
		int GetID () const;
		QRectF GetRect () const;
		QString GetName () const;
	};

	class FormFieldText final : public FormField
							  , public IFormFieldText
	{
		Q_INTERFACES (LC::Monocle::IFormField
				LC::Monocle::IFormFieldText)

		std::shared_ptr<Poppler::FormFieldText> Field_;
	public:
		FormFieldText (std::shared_ptr<Poppler::FormField>);

		FormType GetType () const;
		Qt::Alignment GetAlignment () const;

		QString GetText () const;
		void SetText (const QString&);
		Type GetTextType () const;

		int GetMaximumLength () const;
		bool IsPassword () const;
		bool IsRichText () const;
	};

	class FormFieldChoice final : public FormField
								, public IFormFieldChoice
	{
		Q_INTERFACES (LC::Monocle::IFormField
				LC::Monocle::IFormFieldChoice)

		std::shared_ptr<Poppler::FormFieldChoice> Field_;
	public:
		FormFieldChoice (std::shared_ptr<Poppler::FormField>);

		FormType GetType () const;
		Qt::Alignment GetAlignment () const;

		Type GetChoiceType () const;

		QStringList GetAllChoices () const;

		QList<int> GetCurrentChoices () const;
		void SetCurrentChoices (const QList<int>&);

		QString GetEditChoice () const;
		void SetEditChoice (const QString&);

		bool IsEditable () const;
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

		FormType GetType () const;
		Qt::Alignment GetAlignment () const;

		Type GetButtonType () const;

		QString GetCaption () const;

		bool IsChecked () const;
		void SetChecked (bool);

		QList<int> GetButtonGroup () const;

		LinkAction GetActivationAction () const;
	};
}
}
}
