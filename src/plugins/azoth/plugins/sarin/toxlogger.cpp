/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "toxlogger.h"
#include <QFile>
#include <QDir>
#include <QtDebug>
#include <util/sys/paths.h>

namespace LC
{
namespace Azoth
{
namespace Sarin
{
	ToxLogger::ToxLogger (const QString& name)
	: Name_ { name }
	{
	}

	namespace
	{
		QByteArray LogLevelToMarker (TOX_LOG_LEVEL level)
		{
			switch (level)
			{
			case TOX_LOG_LEVEL_TRACE:
				return "[TRACE] ";
			case TOX_LOG_LEVEL_DEBUG:
				return "[DBG]   ";
			case TOX_LOG_LEVEL_INFO:
				return "[INFO]  ";
			case TOX_LOG_LEVEL_WARNING:
				return "[WARN]  ";
			case TOX_LOG_LEVEL_ERROR:
				return "[ERR]   ";
			}

			qWarning () << Q_FUNC_INFO
					<< "unknown debug level"
					<< level;
			return "[UN]";
		}
	}

	void ToxLogger::Log (TOX_LOG_LEVEL level,
			const char *srcFile, uint32_t line, const char *func,
			const char *message)
	{
		const auto& path = Util::CreateIfNotExists ("azoth/sarin/logs").filePath (Name_ + ".log");
		QFile file { path };
		if (!file.open (QIODevice::WriteOnly | QIODevice::Append))
		{
			qWarning () << Q_FUNC_INFO
					<< "cannot open file"
					<< path
					<< ":"
					<< file.errorString ();
			return;
		}

		file.write (QString { "%1 %2 %4:%3: `%5`" }
				.arg (QString::fromLatin1 (LogLevelToMarker (level)))
				.arg (QString::fromUtf8 (srcFile))
				.arg (line)
				.arg (QString::fromUtf8 (func))
				.arg (QString::fromUtf8 (message))
				.toUtf8 ());
	}
}
}
}
