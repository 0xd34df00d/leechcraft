/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>

class QSqlDatabase;

namespace LC
{
namespace Util
{
namespace oral
{
	template<typename T>
	struct ObjectInfo;

	template<typename T>
	using ObjectInfo_ptr = std::shared_ptr<ObjectInfo<T>>;
}
}
}
