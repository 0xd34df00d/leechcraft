/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "types.h"
#include <util/sll/qtutil.h>

namespace LC::Monocle
{
	const QString TocSectionIdAttr = "section-id"_qs;

	BlockFormat& BlockFormat::operator+= (const BlockFormat& other)
	{
		const auto merge = [&] (auto accessor)
		{
			if (other.*accessor)
				this->*accessor = other.*accessor;
		};

		merge (&BlockFormat::Align_);
		merge (&BlockFormat::MarginLeft_);
		merge (&BlockFormat::MarginTop_);
		merge (&BlockFormat::MarginRight_);
		merge (&BlockFormat::MarginBottom_);
		merge (&BlockFormat::Indent_);
		merge (&BlockFormat::HeadingLevel_);
		merge (&BlockFormat::Background_);
		merge (&BlockFormat::Foreground_);

		return *this;
	}

	CharFormat& CharFormat::operator+= (const CharFormat& other)
	{
		const auto merge = [&] (auto accessor)
		{
			if (other.*accessor)
				this->*accessor = other.*accessor;
		};

		merge (&CharFormat::PointSize_);
		merge (&CharFormat::IsBold_);
		merge (&CharFormat::IsItalic_);
		merge (&CharFormat::IsUnderline_);
		merge (&CharFormat::IsStrikeThrough_);
		merge (&CharFormat::VerticalAlignment_);
		merge (&CharFormat::Background_);
		merge (&CharFormat::Foreground_);

		return *this;
	}

	bool CharFormat::IsEmpty () const
	{
		return PointSize_ ||
				IsBold_ ||
				IsItalic_ ||
				IsUnderline_ ||
				IsStrikeThrough_ ||
				VerticalAlignment_ ||
				Background_ ||
				Foreground_;
	}
}
