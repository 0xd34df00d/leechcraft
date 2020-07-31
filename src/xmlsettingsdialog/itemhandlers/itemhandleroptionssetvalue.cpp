/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "itemhandleroptionssetvalue.h"

namespace LC
{
	void ItemHandlerOptionsSetValue::UpdateValue (QDomElement& element,
			const QVariant& value) const
	{
		QDomNodeList options = element.elementsByTagName ("option");
		for (int i = 0; i < options.size (); ++i)
			options.at (i).toElement ().removeAttribute ("default");

		const QString& optName = value.toString ();
		for (int i = 0; i < options.size (); ++i)
		{
			QDomElement option = options.at (i).toElement ();
			if (option.attribute ("name") == optName)
			{
				option.setAttribute ("default", "true");
				break;
			}
		}
	}

	QVariant ItemHandlerOptionsSetValue::GetValue (const QDomElement& item,
			QVariant value) const
	{
		if (!value.toString ().isEmpty ())
			return value;

		if (item.hasAttribute ("default"))
			return item.attribute ("default");

		auto option = item.firstChildElement ("option");
		while (!option.isNull ())
		{
			if (option.attribute ("default") == "true")
				return option.attribute ("name");

			option = option.nextSiblingElement ("option");
		}

		return value;
	}
}
