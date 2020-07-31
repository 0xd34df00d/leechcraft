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
	class ItemHandlerColor : public ItemHandlerBase
	{
	public:
		using ItemHandlerBase::ItemHandlerBase;

		bool CanHandle (const QDomElement&) const override;
		void Handle (const QDomElement&, QWidget*) override;
		QVariant GetValue (const QDomElement&, QVariant) const override;
		void SetValue (QWidget*, const QVariant&) const override;
		void UpdateValue (QDomElement&, const QVariant&) const override;
	protected:
		QVariant GetObjectValue (QObject*) const override;
	};
}
