/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "mimedetector.h"
#include <QString>
#include <QMimeDatabase>

namespace LC::Util
{
	QByteArray MimeDetector::Detect (const QString& path)
	{
		return QMimeDatabase {}.mimeTypeForFile (path).name ().toUtf8 ();
	}

	QByteArray MimeDetector::operator() (const QString& path)
	{
		return Detect (path);
	}

	QByteArray DetectFileMime (const QString& path)
	{
		return MimeDetector {} (path);
	}
}
