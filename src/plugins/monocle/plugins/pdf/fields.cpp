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

#include "fields.h"

#include <QtDebug>
#include <poppler-form.h>

namespace LeechCraft
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

		qWarning () << Q_FUNC_INFO
				<< "unknown native text type"
				<< Field_->textType ();
		return Type::SingleLine;
	}
}
}
}
