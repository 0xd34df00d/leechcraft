/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "itemhandlerstringgetvalue.h"
#include <QCoreApplication>

namespace LC
{
	QVariant ItemHandlerStringGetValue::GetValue (const QDomElement& item,
			QVariant) const
	{
		const auto& context = XSD_->GetBasename ();

		const auto& defChild = item.firstChildElement ("default");
		if (!defChild.isNull ())
			return QCoreApplication::translate (qPrintable (context),
					defChild.text ().toUtf8 ().constData ());

		auto def = item.attribute ("default");
		if (item.attribute ("translatable") == "true")
			def = QCoreApplication::translate (qPrintable (context),
					def.toUtf8 ().constData ());
		return def;
	}
}
