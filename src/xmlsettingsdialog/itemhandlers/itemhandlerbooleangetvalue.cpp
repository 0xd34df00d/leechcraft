/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2010  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "itemhandlerbooleangetvalue.h"

namespace LeechCraft
{
	ItemHandlerBooleanGetValue::ItemHandlerBooleanGetValue ()
	{
	}

	ItemHandlerBooleanGetValue::~ItemHandlerBooleanGetValue ()
	{
	}

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
};
