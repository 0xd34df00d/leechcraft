/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "rplisupportparser.h"
#include <util/sll/qtutil.h>

namespace LC::Azoth::Acetamide
{
	std::optional<QHash<QString, QString>> ParseISupportReply (const QString& reply)
	{
		const auto firstSpace = reply.indexOf (' ');
		if (firstSpace < 0)
			return {};

		const auto& withoutNick = reply.midRef (firstSpace + 1);
		const auto endMarker = u" :are supported by this server"_qsv;
		if (!withoutNick.endsWith (endMarker))
			return {};

		const auto& features = withoutNick.chopped (static_cast<int> (endMarker.size ()));

		QHash<QString, QString> result;
		for (const auto& feature : features.split (' ', Qt::SkipEmptyParts))
		{
			const bool isNegated = feature [0] == '-';
			const auto eqPos = feature.indexOf ('=');

			auto& featureVal = result [feature.left (eqPos).toString ()];
			if (isNegated)
				featureVal = "false"_ql;
			else if (eqPos < 0)
				featureVal = "true"_ql;
			else
				featureVal = feature.mid (eqPos + 1).toString ();
		}
		return result;
	}
}
