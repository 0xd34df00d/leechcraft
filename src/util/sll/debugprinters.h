/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "sllconfig.h"
#include <QDebug>
#include <QDomDocument>
#include "either.h"

UTIL_SLL_API QDebug operator<< (QDebug, const QDomDocument::ParseResult&);

namespace LC::Util
{
	template<typename T>
	concept QDebuggable = requires (T t, QDebug out) { out << t; };

	template<QDebuggable L, QDebuggable R>
	QDebug operator<< (QDebug out, const Either<L, R>& either)
	{
		QDebugStateSaver saver { out };
		Visit (either,
				[&out] (const L& l) { out.nospace () << "L { " << l << " }"; },
				[&out] (const R& r) { out.nospace () << "R { " << r << " }"; });
		return out;
	}
}
