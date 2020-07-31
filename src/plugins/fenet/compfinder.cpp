/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "compfinder.h"

namespace LC::Fenet
{
	CompFinder::CompFinder (QObject *parent)
	: FinderBase (parent)
	{
		Find ("compositing");
	}

	CompInfo CompFinder::GetInfo (const QString&, const QStringList& execs, const QVariantMap& map) const
	{
		CompInfo info
		{
			{},
			{},
			map ["name"].toString (),
			map ["comment"].toString (),
			execs
		};

		for (const auto& item : map ["flags"].toList ())
		{
			const auto& flagMap = item.toMap ();
			info.Flags_.append ({ flagMap ["param"].toString (), flagMap ["desc"].toString () });
		}

		for (const auto& item : map ["params"].toList ())
		{
			const auto& pMap = item.toMap ();
			info.Params_.append ({
					pMap ["param"].toString (),
					pMap ["desc"].toString (),

					pMap ["default"].toDouble (),

					pMap.value ("min", 0).toDouble (),
					pMap.value ("max", 0).toDouble ()
				});
		}

		return info;
	}
}
