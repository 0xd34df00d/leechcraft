/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <utility>
#include <QString>
#include "sllconfig.h"

namespace LC::Util
{
	/** @brief A text to be shown as it is.
	 */
	struct PlainTextString
	{
		QString Text_;
	};

	/** @brief A text in Qt's rich text markup.
	 *
	 * Making one from a string states that the string already is markup,
	 * with whatever data it carries escaped. A `PlainTextString` converts
	 * to the markup showing it, line breaks included.
	 */
	class RichTextString
	{
		QString Html_;
	public:
		RichTextString () = default;

		explicit RichTextString (QString html)
		: Html_ { std::move (html) }
		{
		}

		UTIL_SLL_API explicit (false) RichTextString (const PlainTextString& plain);

		[[nodiscard]] QString ToHtml () const
		{
			return Html_;
		}

		bool operator== (const RichTextString&) const = default;
	};
}
