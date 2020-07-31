/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "itemhandlerstringsetvalue.h"

namespace LC
{
	/** This is for those whose value is boolean - on/off or true/false.
	 * These are:
	 * - checkbox
	 * - groupbox with "checkable" attribute in true
	 */
	class ItemHandlerBooleanGetValue : public ItemHandlerStringSetValue
	{
	public:
		using ItemHandlerStringSetValue::ItemHandlerStringSetValue;

		QVariant GetValue (const QDomElement& element,
				QVariant value) const override;
	};
}
