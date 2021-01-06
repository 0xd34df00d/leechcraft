/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "buildtagstree.h"
#include <QDomDocument>

namespace LC::Util
{
	QDomElement BuildTagsTree (QStringList tags,
			QDomNode& node,
			QDomDocument& document,
			const QString& elementName,
			const std::function<QString (QDomElement)>& tagGetter,
			const std::function<void (QDomElement, QString)>& tagSetter)
	{
		if (tags.isEmpty ())
			return node.toElement ();

		const auto& tag = tags.value (0);
		tags.removeFirst ();

		auto elements = node.childNodes ();
		for (int i = 0; i < elements.size (); ++i)
		{
			auto elem = elements.at (i).toElement ();
			if (tagGetter (elem) == tag)
				return BuildTagsTree (tags, elem,
						document, elementName,
						tagGetter, tagSetter);
		}

		auto result = document.createElement (elementName);
		tagSetter (result, tag);
		node.appendChild (result);
		return BuildTagsTree (tags, result,
				document, elementName,
				tagGetter, tagSetter);
	}
}
