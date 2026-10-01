/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "logging.h"
#include <QMessageLogger>

namespace LC::Util
{
	QMessageLogger LogAt (std::source_location loc)
	{
		return { loc.file_name (), static_cast<int> (loc.line ()), loc.function_name () };
	}
}
