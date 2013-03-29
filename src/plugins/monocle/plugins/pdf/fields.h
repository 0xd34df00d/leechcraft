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
}

namespace LeechCraft
{
namespace Monocle
{
namespace PDF
{
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

		QString GetText () const;
		void SetText (const QString&);
		Type GetTextType () const;
	};
}
}
}
