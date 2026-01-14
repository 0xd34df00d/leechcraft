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
#include <util/sll/qtutil.h>
#include <util/sys/paths.h>

namespace LC::Azoth::Sarin
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
				return "[TRACE] "_qba;
			case TOX_LOG_LEVEL_DEBUG:
				return "[DBG]   "_qba;
			case TOX_LOG_LEVEL_INFO:
				return "[INFO]  "_qba;
			case TOX_LOG_LEVEL_WARNING:
				return "[WARN]  "_qba;
			case TOX_LOG_LEVEL_ERROR:
				return "[ERR]   "_qba;
			}

			qWarning () << "unknown debug level" << level;
			return "[UN] "_qba;
		}
	}

	void ToxLogger::Log (TOX_LOG_LEVEL level,
			const char *srcFile, uint32_t line, const char *func,
			const char *message)
	{
		const auto& logStr = "%1 %2 %4:%3: `%5`"_qs
				.arg (LogLevelToMarker (level), srcFile, QByteArray::number (line), func, message)
				.toUtf8 ();
		const auto& path = Util::CreateIfNotExists ("azoth/sarin/logs"_qs).filePath (Name_ + ".log"_qs);
		if (QFile file { path };
			!file.open (QIODevice::WriteOnly | QIODevice::Append))
		{
			qWarning () << "cannot open file" << path << ":" << file.errorString ();
			qWarning () << logStr;
		}
		else
			file.write (logStr);
	}
}
