/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "util.h"
#include <QString>
#include <Wt/WString.h>

namespace LC
{
namespace Aggregator
{
namespace WebAccess
{
	Wt::WString ToW (const QString& str)
	{
		return Wt::WString (str.toUtf8 ().constData (), Wt::CharEncoding::UTF8);
	}
}
}
}
