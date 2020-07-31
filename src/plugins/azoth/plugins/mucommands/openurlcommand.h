/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <interfaces/core/icoreproxy.h>
#include <interfaces/structures.h>
#include <interfaces/azoth/iprovidecommands.h>

class QString;

namespace LC
{
namespace Azoth
{
class IProxyObject;
class ICLEntry;

namespace MuCommands
{
	StringCommandResult ListUrls (IProxyObject*, ICLEntry*, const QString&);

	CommandResult_t OpenUrl (const ICoreProxy_ptr&, IProxyObject*, ICLEntry*, const QString&, TaskParameters);
}
}
}
