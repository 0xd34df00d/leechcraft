/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "serialization.h"
#include <QtDebug>
#include <util/sll/logging.h>

namespace LC::Util
{
	namespace detail
	{
		void ReportBadVersion (std::source_location loc, qint64 min, qint64 max, qint64 version)
		{
			LogAt (loc).warning () << "unsupported version" << version << "; expected [" << min << " .. " << max << "]";
		}

		void ReportBadStreamStatus (std::source_location loc, QDataStream::Status status)
		{
			LogAt (loc).warning () << "bad stream status" << status;
		}
	}

	template UTIL_SYS_API std::optional<quint8> EnsureVersion<quint8> (QDataStream&, quint8, quint8, std::source_location);
	template UTIL_SYS_API std::optional<qint8> EnsureVersion<qint8> (QDataStream&, qint8, qint8, std::source_location);
	template UTIL_SYS_API std::optional<quint16> EnsureVersion<quint16> (QDataStream&, quint16, quint16, std::source_location);
}
