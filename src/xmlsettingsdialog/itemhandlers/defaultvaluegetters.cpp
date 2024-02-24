/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "defaultvaluegetters.h"
#include <QCoreApplication>
#include <QDomElement>
#include <QVariant>
#include <util/sll/qtutil.h>

namespace LC
{
	QVariant GetDefaultBooleanValue (const QString& def)
	{
		return def == "on"_ql || def == "true"_ql;
	}

	QVariant GetDefaultStringValue (const QString& def, const QByteArray& trCtx)
	{
		return QCoreApplication::translate (trCtx.constData (), def.toUtf8 ().constData ());
	}
}
