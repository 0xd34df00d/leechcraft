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
	QVariant GetDefaultBooleanValue (const QDomElement& item)
	{
		if (item.hasAttribute ("default"_qs))
		{
			const auto& attrVal = item.attribute ("default"_qs);
			return attrVal == "on"_ql || attrVal == "true"_ql;
		}
		if (item.hasAttribute ("state"_qs))
			return item.attribute ("state"_qs) == "on"_ql;

		return {};
	}

	QVariant GetDefaultStringValue (const QDomElement& item, const QByteArray& trCtx)
	{
		const auto& defChild = item.firstChildElement ("default"_qs);
		if (!defChild.isNull ())
			return QCoreApplication::translate (trCtx.constData (),
					defChild.text ().toUtf8 ().constData ());

		auto def = item.attribute ("default"_qs);
		if (item.attribute ("translatable"_qs) == "true"_ql)
			def = QCoreApplication::translate (trCtx.constData (),
					def.toUtf8 ().constData ());
		return def;
	}
}
