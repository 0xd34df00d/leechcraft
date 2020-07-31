/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "urlaccessor.h"

namespace LC
{
namespace Util
{
	UrlAccessor::UrlAccessor (const QUrl& url)
	: Url_ { url }
	{
	}

	QString UrlAccessor::operator[] (const QString& key) const
	{
		return Url_.queryItemValue (key);
	}

	const UrlAccessor::value_type& UrlAccessor::last () const
	{
		return Url_.queryItems ().last ();
	}
}
}
