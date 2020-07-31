/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "wmfinder.h"

namespace LC::Fenet
{
	WMFinder::WMFinder (QObject *parent)
	: FinderBase (parent)
	{
		Find ("wms");
	}

	WMInfo WMFinder::GetInfo (const QString& filePath,
			const QStringList& execNames, const QVariantMap& varmap) const
	{
		auto session = filePath;
		session.chop (5);
		session += ".sh";
		return
		{
			varmap ["name"].toString (),
			varmap ["comment"].toString (),
			execNames,
			session,
			varmap ["compositing"].toBool ()
		};
	}
}
