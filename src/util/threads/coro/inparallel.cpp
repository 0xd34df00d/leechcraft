/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "inparallel.h"
#include <QtDebug>
#include "context.h"

namespace LC::Util::detail
{
	void HandleSubsequentException ()
	{
		try
		{
			throw;
		}
		catch (const ContextDeadException&)
		{
			// "normal" child coro death, nothing to do here
		}
		catch (const std::exception& e)
		{
			qWarning () << "dropping a subsequent exception:" << e.what ();
		}
		catch (...)
		{
			qWarning () << "dropping a subsequent non-std::exception";
		}
	}
}
