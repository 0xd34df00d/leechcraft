/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QMutex>
#include "attributes.h"

namespace LC::Util
{
	class CAPABILITY("mutex") Mutex
	{
		QMutex M_;
	public:
		explicit Mutex () = default;

		void lock () ACQUIRE ()
		{
			M_.lock ();
		}

		void unlock () RELEASE ()
		{
			M_.unlock ();
		}

		QMutex& GetMutex ()
		{
			return M_;
		}
	};

	class SCOPED_LOCKABLE MutexLocker final
	{
		Mutex& M_;
	public:
		explicit MutexLocker (Mutex& m) ACQUIRE (m)
		: M_ { m }
		{
			M_.lock ();
		}

		~MutexLocker () RELEASE ()
		{
			M_.unlock ();
		}
	};
}
