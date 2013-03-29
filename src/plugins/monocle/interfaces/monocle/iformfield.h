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

#include <memory>
#include <QtPlugin>

class QRectF;

namespace LeechCraft
{
namespace Monocle
{
	enum class FormType
	{
		Text,
		Choice,
		Button
	};

	class IFormField
	{
	public:
		virtual ~IFormField () {}

		virtual FormType GetType () const = 0;

		virtual int GetID () const = 0;

		virtual QString GetName () const = 0;

		virtual QRectF GetRect () const = 0;

		virtual Qt::Alignment GetAlignment () const = 0;
	};

	typedef std::shared_ptr<IFormField> IFormField_ptr;

	class IFormFieldText
	{
	public:
		enum class Type
		{
			SingleLine,
			Multiline,
			File
		};

		virtual ~IFormFieldText () {}

		virtual QString GetText () const = 0;
		virtual void SetText (const QString&) = 0;

		virtual Type GetTextType () const = 0;

		virtual int GetMaximumLength () const = 0;

		virtual bool IsPassword () const = 0;

		virtual bool IsRichText () const = 0;
	};

	class IFormFieldChoice
	{
	public:
		enum class Type
		{
			Combobox,
			ListBox
		};

		virtual ~IFormFieldChoice () {}

		virtual Type GetChoiceType () const = 0;

		virtual QStringList GetAllChoices () const = 0;

		virtual QList<int> GetCurrentChoices () const = 0;
		virtual void SetCurrentChoices (const QList<int>&) = 0;

		virtual QString GetEditChoice () const = 0;
		virtual void SetEditChoice (const QString&) = 0;

		virtual bool IsEditable () const = 0;
	};

	class IFormFieldButton
	{
	public:
		virtual ~IFormFieldButton () {}
	};
}
}

Q_DECLARE_INTERFACE (LeechCraft::Monocle::IFormField,
		"org.LeechCraft.Monocle.IFormField/1.0");
Q_DECLARE_INTERFACE (LeechCraft::Monocle::IFormFieldText,
		"org.LeechCraft.Monocle.IFormFieldText/1.0");
Q_DECLARE_INTERFACE (LeechCraft::Monocle::IFormFieldChoice,
		"org.LeechCraft.Monocle.IFormFieldChoice/1.0");
Q_DECLARE_INTERFACE (LeechCraft::Monocle::IFormFieldButton,
		"org.LeechCraft.Monocle.IFormFieldButton/1.0");
