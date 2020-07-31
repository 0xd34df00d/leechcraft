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
	class ItemHandlerNoValue : public ItemHandlerBase
	{
	public:
		using ItemHandlerBase::ItemHandlerBase;

		QVariant GetValue (const QDomElement&, QVariant) const;
		void SetValue (QWidget*, const QVariant&) const;
		void UpdateValue (QDomElement&, const QVariant&) const;
	protected:
		QVariant GetObjectValue (QObject*) const;
	};
}
