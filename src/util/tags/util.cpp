/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "util.h"
#include <QString>

namespace LC::Util
{
	QString GetDefaultTagsSeparator ()
	{
		return QStringLiteral ("; ");
	}

	QStringList SplitTags (const QString& text, const QString& separator)
	{
		const auto& trimmedSeparator = separator.trimmed ();
		const auto& effectiveSeparator = trimmedSeparator.isEmpty () ? separator : trimmedSeparator;

		QStringList result;
		for (const auto& part : QStringView { text }.split (effectiveSeparator, Qt::SkipEmptyParts))
			if (const auto& tag = part.trimmed (); !tag.isEmpty ())
				result << tag.toString ();
		return result;
	}
}
