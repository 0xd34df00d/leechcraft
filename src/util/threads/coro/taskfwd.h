/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

namespace LC::Util
{
	namespace detail
	{
		template<typename>
		struct NoExtensions {};
	}

	template<typename R, template<typename> typename Extensions = detail::NoExtensions>
	class Task;
}
