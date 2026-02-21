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

		switch (info.Kind_)
		{
		case ProcessKind::Download:
		case ProcessKind::Upload:
			return QObject::tr ("%1 of %2").arg (Util::MakePrettySize (done), Util::MakePrettySize (total));
		case ProcessKind::Generic:
			return QObject::tr ("%1 of %2").arg (done).arg (total);
		}
		return {};
	}
}
