/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <boost/optional.hpp>

namespace LC::Util::oral::detail
{
	struct CachedFieldsData
	{
		QString Table_;

		QStringList Fields_;
		QStringList QualifiedFields_;
		QStringList BoundFields_;
	};

}
