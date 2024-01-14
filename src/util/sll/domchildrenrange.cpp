/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "domchildrenrange.h"
#include <QVector>

namespace LC::Util
{
	QVector<QDomElement> DomDescendants (const QDomElement& parent, const QString& tag)
	{
		const auto& allElems = parent.elementsByTagName (tag);

		QVector<QDomElement> result;
		result.reserve (allElems.size ());
		for (int i = 0; i < allElems.size (); ++i)
			result << allElems.at (i).toElement ();
		return result;
	}
}
