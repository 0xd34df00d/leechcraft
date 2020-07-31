/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QtGlobal>
#include <QUrlQuery>
#include "sllconfig.h"

namespace LC
{
namespace Util
{
	class UTIL_SLL_API UrlAccessor
	{
		const QUrlQuery Url_;
	public:
		using value_type = typename decltype (Url_.queryItems ())::value_type;

		UrlAccessor (const QUrl&);

		QString operator[] (const QString&) const;

		const value_type& last () const;
	};
}
}
