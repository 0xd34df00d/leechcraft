/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "ipluginloader.h"
#include <QLibrary>
#include <QtDebug>

namespace LC
{
namespace Loaders
{
	qint64 GetLibAPILevel (const QString& file)
	{
		if (file.isEmpty ())
			return static_cast<quint64> (-1);

		QLibrary library (file);
		if (!library.load ())
			return static_cast<quint64> (-1);

		typedef quint64 (*APIVersion_t) ();
		auto getter = reinterpret_cast<APIVersion_t> (library.resolve ("GetAPILevels"));
		return getter ? getter () : static_cast<quint64> (-1);
	}
}
}
