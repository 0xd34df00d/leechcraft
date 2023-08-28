/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "context.h"

namespace LC::Util
{
	namespace
	{
		auto MakeDeadObjectMessage (const detail::DeadObjectInfo& info)
		{
			const std::string prefix = "coroutine's context object " + info.ClassName_;
			if (info.ObjectName_.isEmpty ())
				return prefix + " died";
			else
				return prefix + " (" + info.ObjectName_.toStdString () + ") died";
		}
	}

	ContextDeadException::ContextDeadException (const detail::DeadObjectInfo& info)
	: std::runtime_error { MakeDeadObjectMessage (info) }
	{
	}

	namespace detail
	{
		void CheckDeadObjects (const QVector<DeadObjectInfo>& deadObjects)
		{
			if (!deadObjects.isEmpty ())
				throw ContextDeadException { deadObjects.front () };
		}
	}
}
