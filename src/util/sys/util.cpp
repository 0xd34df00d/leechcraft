/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "util.h"
#include <QString>
#include <QImageWriter>

namespace LC
{
namespace Util
{
	bool HasSupportedImageExtension (const QString& filename)
	{
		if (filename.count ('.') < 1)
			return false;

		const auto& ext = filename.section ('.', -1, -1);
		const auto& formats = QImageWriter::supportedImageFormats ();
		return std::any_of (formats.begin (), formats.end (),
				[&ext] (const QByteArray& format)
					{ return !QString::compare (ext, format, Qt::CaseInsensitive); });
	}

	bool IsOSXLoadFromBundle ()
	{
#if defined (Q_OS_MAC) && !defined (USE_UNIX_LAYOUT)
		return qgetenv ("LC_BUNDLE_DIRECT_LOAD") != "1";
#else
		return false;
#endif
	}
}
}
