/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "accountthread.h"
#include <QtDebug>
#include "account.h"
#include "accountthreadworker.h"
#include "core.h"

namespace LC
{
namespace Snails
{
	GenericExceptionWrapper::GenericExceptionWrapper (const std::exception_ptr& ptr)
	{
		if (!ptr)
		{
			Msg_ = "no exception information";
			return;
		}

		try
		{
			std::rethrow_exception (ptr);
		}
		catch (const std::exception& e)
		{
			Msg_ = std::string { "generic exception of type `" } + typeid (e).name () +
					"`: `" + e.what () + "`";
		}
	}

	const char* GenericExceptionWrapper::what () const noexcept
	{
		return Msg_.c_str ();
	}

	namespace detail
	{
		void ReconnectATW (AccountThreadWorker *w)
		{
			w->Disconnect ();
		}
	}

	size_t AccountThread::GetQueueSize ()
	{
		return IsRunning_ + WorkerThread::GetQueueSize ();
	}
}
}
