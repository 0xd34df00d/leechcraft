/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "itemhandlernovalue.h"

namespace LC
{
	QVariant ItemHandlerNoValue::GetValue (const QDomElement&, QVariant) const
	{
		return QVariant ();
	}

	void ItemHandlerNoValue::SetValue (QWidget*, const QVariant&) const
	{
	}

	QVariant ItemHandlerNoValue::GetObjectValue (QObject*) const
	{
		return QVariant ();
	}
}
