/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "util.h"
#include <QObject>
#include <QList>
#include <QPair>
#include <QVariant>
#include <interfaces/ihavetabs.h>

namespace LC::TabSessManager
{
	QList<QPair<QByteArray, QVariant>> GetSessionProps (QObject *tab)
	{
		QList<QPair<QByteArray, QVariant>> props;
		for (const auto& propName : tab->dynamicPropertyNames ())
		{
			if (!propName.startsWith ("SessionData/"))
				continue;

			props.append ({ propName, tab->property (propName) });
		}
		return props;
	}

	bool IsGoodSingleTC (const TabClassInfo& tc)
	{
		return tc.Features_ & TabFeature::TFSingle && tc.Features_ & TabFeature::TFOpenableByRequest;
	}
}
