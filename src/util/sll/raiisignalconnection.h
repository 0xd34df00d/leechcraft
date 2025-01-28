/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QMetaObject>
#include "sllconfig.h"

namespace LC::Util
{
	class UTIL_SLL_API RaiiSignalConnection
	{
		QMetaObject::Connection Conn_;
	public:
		explicit RaiiSignalConnection (QMetaObject::Connection conn);
		~RaiiSignalConnection ();

		RaiiSignalConnection () = default;
		RaiiSignalConnection (const RaiiSignalConnection&) = delete;
		RaiiSignalConnection (RaiiSignalConnection&&) noexcept;

		RaiiSignalConnection& operator= (const RaiiSignalConnection&) = delete;
		RaiiSignalConnection& operator= (RaiiSignalConnection&&) noexcept;
		RaiiSignalConnection& operator= (QMetaObject::Connection);

		QMetaObject::Connection Release () &&;
	};
}
