/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "pls.h"
#include <algorithm>
#include <QSettings>
#include "commonpl.h"

namespace LC
{
namespace LMP
{
namespace PLS
{
	QList<RawReadData> Read (const QString& path)
	{
		QList<RawReadData> result;

		QSettings settings (path, QSettings::IniFormat);
		settings.beginGroup ("playlist");

		const int numFiles = settings.value ("NumberOfEntries").toInt ();
		for (int i = 1; i <= numFiles; ++i)
		{
			const auto& str = settings.value ("File" + QString::number (i)).toString ();
			if (!str.isEmpty ())
				result.append ({ str, {} });
		}

		settings.endGroup ();

		return result;
	}

	Playlist Read2Sources (const QString& path)
	{
		return CommonRead2Sources ({ { "pls" }, path, Read });
	}
}
}
}
