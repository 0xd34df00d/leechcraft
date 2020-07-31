/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "itemhandlerbase.h"
#include <QtDebug>
#include "../basesettingsmanager.h"

namespace LC
{
	ItemHandlerBase::ItemHandlerBase (Util::XmlSettingsDialog *xsd)
	: XSD_ { xsd }
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
		const auto& propertyName = sender ()->objectName ();
		const auto& value = GetObjectValue (sender ());
		ChangedProperties_ [propertyName] = value;

		XSD_->GetManagerObject ()->OptionSelected (propertyName.toLatin1 (), value);
	}
}
