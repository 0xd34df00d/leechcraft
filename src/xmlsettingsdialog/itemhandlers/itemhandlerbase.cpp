/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#include "itemhandlerbase.h"

namespace LeechCraft
{
	Util::XmlSettingsDialog *ItemHandlerBase::XSD_ = 0;

	void ItemHandlerBase::SetXmlSettingsDialog (Util::XmlSettingsDialog *xsd)
	{
		XSD_ = xsd;
	}

	ItemHandlerBase::ItemHandlerBase ()
	{
	}

	ItemHandlerBase::~ItemHandlerBase ()
	{
	}

	ItemHandlerBase::Prop2NewValue_t ItemHandlerBase::GetChangedProperties () const
	{
		return ChangedProperties_;
	}

	void ItemHandlerBase::ClearChangedProperties ()
	{
		ChangedProperties_.clear ();
	}

	void ItemHandlerBase::updatePreferences ()
	{
		QString propertyName = sender ()->objectName ();
		ChangedProperties_ [propertyName] = GetValue (sender ());
	}
};
