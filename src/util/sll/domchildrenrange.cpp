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
	namespace
	{
		void DomDescendants (const QDomElement& parent, const QString& tag, QVector<QDomElement>& result)
		{
			for (const auto& elem : DomChildren (parent, {}))
			{
				if (elem.tagName () == tag)
					result << elem;

				DomDescendants (elem, tag, result);
			}
		}
	}

	QVector<QDomElement> DomDescendants (const QDomElement& parent, const QString& tag)
	{
		const auto& allElems = parent.elementsByTagName (tag);
		const auto elemsCount = allElems.size ();

		QVector<QDomElement> result;
		result.reserve (elemsCount);

		// QDomNodeList operations are slower than explicit recursion for bigger lists,
		// and this is a somewhat empirical threshold.
		constexpr auto countThreshold = 200;

		if (elemsCount < countThreshold)
			for (int i = 0; i < allElems.size (); ++i)
				result << allElems.at (i).toElement ();
		else
			DomDescendants (parent, tag, result);

		return result;
	}
}
