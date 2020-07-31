/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "itemhandlerbase.h"

namespace LC
{
	/** This is the base handler for those items whose values are
	 * represented by a plain string (created from QVariant::toString())
	 * in the XML format.
	 * These are:
	 * - lineedit
	 * - spinbox
	 * - doublespinbox
	 * - checkbox
	 * - groupbox
	 * - path
	 */
	class ItemHandlerStringSetValue : public ItemHandlerBase
	{
	public:
		using ItemHandlerBase::ItemHandlerBase;

		void UpdateValue (QDomElement& element,
				const QVariant& value) const override;
	};
}
