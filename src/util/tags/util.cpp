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

	QString GetTagsDelimiter (const QString& separator)
	{
		const auto& trimmed = separator.trimmed ();
		return trimmed.isEmpty () ? separator : trimmed;
	}

	QStringList SplitTags (const QString& text, const QString& separator)
	{
		QStringList result;
		for (const auto& part : QStringView { text }.split (GetTagsDelimiter (separator), Qt::SkipEmptyParts))
			if (const auto& tag = part.trimmed (); !tag.isEmpty ())
				result << tag.toString ();
		return result;
	}
}
