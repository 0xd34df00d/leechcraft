/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "core.h"
#include "linuxplatformbackend.h"

namespace LC
{
namespace Lemon
{
	Core::Core ()
#ifdef Q_OS_LINUX
	: Backend_ (new LinuxPlatformBackend)
#endif
	{
	}

	Core& Core::Instance ()
	{
		static Core c;
		return c;
	}

	void Core::Release ()
	{
		Backend_.reset ();
	}

	void Core::SetProxy (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;
	}

	ICoreProxy_ptr Core::GetProxy () const
	{
		return Proxy_;
	}

	PlatformBackend_ptr Core::GetPlatformBackend () const
	{
		return Backend_;
	}
}
}
