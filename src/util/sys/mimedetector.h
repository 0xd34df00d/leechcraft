/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include "sysconfig.h"

class QString;
class QByteArray;

namespace LC
{
namespace Util
{
	class MimeDetectorImpl;

	class MimeDetector
	{
		const std::shared_ptr<MimeDetectorImpl> Impl_;
	public:
		UTIL_SYS_API MimeDetector ();

		UTIL_SYS_API QByteArray Detect (const QString&);
		UTIL_SYS_API QByteArray operator() (const QString&);
	};

	UTIL_SYS_API QByteArray DetectFileMime (const QString&);
}
}
