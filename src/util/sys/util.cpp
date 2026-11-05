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

#ifdef Q_OS_LINUX
#include <sys/prctl.h>
#elifdef Q_OS_FREEBSD
#include <sys/procctl.h>
#endif

namespace LC::Util
{
	bool HasSupportedImageExtension (const QString& filename)
	{
		if (filename.count ('.') < 1)
			return false;

		const auto& ext = filename.section ('.', -1, -1);
		const auto& formats = QImageWriter::supportedImageFormats ();
		return std::any_of (formats.begin (), formats.end (),
				[&ext] (const QByteArray& format)
					{ return !QString::compare (ext, QString::fromUtf8 (format), Qt::CaseInsensitive); });
	}

	bool IsOSXLoadFromBundle ()
	{
#if defined (Q_OS_MAC) && !defined (USE_UNIX_LAYOUT)
		return qgetenv ("LC_BUNDLE_DIRECT_LOAD") != "1";
#else
		return false;
#endif
	}

	bool SetParentDeathSignal ([[maybe_unused]] int signum)
	{
#ifdef Q_OS_LINUX
		return prctl (PR_SET_PDEATHSIG, signum) == 0;
#elifdef Q_OS_FREEBSD
		return procctl (P_PID, 0, PROC_PDEATHSIG_CTL, &signum) == 0;
#else
		return false;
#endif
	}
}
