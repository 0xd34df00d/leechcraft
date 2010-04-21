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

#include "itemhandleroptionssetvalue.h"

namespace LeechCraft
{
	ItemHandlerOptionsSetValue::~ItemHandlerOptionsSetValue ()
	{
	}

	void ItemHandlerOptionsSetValue::UpdateValue (QDomElement& element,
			const QVariant& value) const
	{
		QDomNodeList options = element.elementsByTagName ("option");
		for (int i = 0; i < options.size (); ++i)
			options.at (i).toElement ().removeAttribute ("default");

		QString optName = value.toString ();
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
		if (value.isNull () ||
				value.toString ().isEmpty ())
		{
			if (item.hasAttribute ("default"))
				value = item.attribute ("default");
			else
			{
				QDomElement option = item.firstChildElement ("option");
				while (!option.isNull ())
				{
					if (option.attribute ("default") == "true")
					{
						value = option.attribute ("name");
						break;
					}
					option = option.nextSiblingElement ("option");
				}
			}
		}

		return value;
	}
};
