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
}
