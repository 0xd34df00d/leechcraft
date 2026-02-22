/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "util.h"
#include <QModelIndex>
#include <util/util.h>

namespace LC::Summary
{
	QString MakeProgressString (const ProcessInfo& info, qlonglong done, qlonglong total, const QModelIndex& srcIdx)
	{
		if (const auto& customData = srcIdx.data (+JobHolderProcessRole::ProgressCustomText).toString ();
			!customData.isNull ())
			return customData;

		const auto percentage = std::max (done, 0LL) / std::max (static_cast<double> (total), 1.) * 100;
		const auto& pattern = QObject::tr ("%1 of %2 (%3%)");
		const auto& absValuesSubstitued = [&]
		{
			switch (info.Kind_)
			{
			case ProcessKind::Download:
			case ProcessKind::Upload:
				return pattern.arg (Util::MakePrettySize (done), Util::MakePrettySize (total));
			case ProcessKind::Generic:
				return pattern.arg (done).arg (total);
			}
		} ();
		return absValuesSubstitued.arg (percentage, 0, 'f', 1);
	}
}
