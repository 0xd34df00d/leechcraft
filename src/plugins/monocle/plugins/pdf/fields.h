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

namespace LeechCraft
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
		Q_INTERFACES (LeechCraft::Monocle::IFormField)

		std::shared_ptr<Poppler::FormField> BaseField_;
	protected:
		FormField (std::shared_ptr<Poppler::FormField>);
	public:
		int GetID () const;
		QRectF GetRect () const;
		QString GetName () const;
	};

	class FormFieldText : public FormField
						, public IFormFieldText
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Monocle::IFormField
				LeechCraft::Monocle::IFormFieldText)

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

	class FormFieldChoice : public FormField
						  , public IFormFieldChoice
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Monocle::IFormField
				LeechCraft::Monocle::IFormFieldChoice)

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

	class FormFieldButton : public FormField
						  , public IFormFieldButton
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Monocle::IFormField
				LeechCraft::Monocle::IFormFieldButton)

		std::shared_ptr<Poppler::FormFieldButton> Field_;
		Document *Doc_;
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

		void HandleActivated ();
	};
}
}
}
