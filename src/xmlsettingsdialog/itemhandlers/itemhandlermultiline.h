/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "itemhandlerstringgetvalue.h"

namespace LC
{
	class ItemHandlerMultiLine : public ItemHandlerStringGetValue
	{
	public:
		using ItemHandlerStringGetValue::ItemHandlerStringGetValue;

		bool CanHandle (const QDomElement&) const;
		void Handle (const QDomElement&, QWidget*);
		void SetValue (QWidget*, const QVariant&) const;
	protected:
		QVariant GetObjectValue (QObject*) const;
	};
}
