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
	/* This is the base handle for those items whose values are
	 * represented by a list of options:
	 * - radio
	 * - combobox
	 */
	class ItemHandlerOptionsSetValue : public ItemHandlerBase
	{
	public:
		using ItemHandlerBase::ItemHandlerBase;

		void UpdateValue (QDomElement&, const QVariant& value) const override;
		QVariant GetValue (const QDomElement& element, QVariant value) const override;
	};
}
