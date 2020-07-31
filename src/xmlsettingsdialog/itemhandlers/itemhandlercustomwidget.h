/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "itemhandlernovalue.h"

namespace LC
{
	class ItemHandlerCustomWidget : public ItemHandlerNoValue
	{
	public:
		using ItemHandlerNoValue::ItemHandlerNoValue;

		bool CanHandle (const QDomElement&) const override;
		void Handle (const QDomElement&, QWidget*) override;
	};
}
