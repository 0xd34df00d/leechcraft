/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "itemhandlerbooleangetvalue.h"

namespace LC
{
	QVariant ItemHandlerBooleanGetValue::GetValue (const QDomElement& item,
			QVariant value) const
	{
		if (!value.isValid () ||
				value.isNull () ||
				value.toString () == "on" ||
				value.toString () == "off" ||
				value.toString () == "true" ||
				value.toString () == "false")
		{
			if (item.hasAttribute ("default"))
				value = (item.attribute ("default") == "on" ||
						item.attribute ("default") == "true");
			else
				value = (item.attribute ("state") == "on");
		}
		return value;
	}
}
