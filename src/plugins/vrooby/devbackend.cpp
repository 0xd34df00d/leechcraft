/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "devbackend.h"
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/structures.h>

namespace LC
{
namespace Vrooby
{
	DevBackend::DevBackend (const ICoreProxy_ptr& proxy)
	: Proxy_ { proxy }
	{
	}

	void DevBackend::HandleEntity (const Entity& e)
	{
		Proxy_->GetEntityManager ()->HandleEntity (e);
	}
}
}
