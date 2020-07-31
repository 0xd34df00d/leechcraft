/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QSet>
#include <QList>

namespace LC::Util
{
	template<typename T>
#if QT_VERSION >= QT_VERSION_CHECK (5, 14, 0)
	auto AsSet (const T& cont)
#else
	auto AsSet (const QList<T>& cont)
#endif
	{
#if QT_VERSION >= QT_VERSION_CHECK (5, 14, 0)
		return QSet (cont.begin (), cont.end ());
#else
		return QSet<T>::fromList (cont);
#endif
	}
}
