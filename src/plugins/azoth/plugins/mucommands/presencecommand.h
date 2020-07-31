/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

class QString;

namespace LC
{
namespace Azoth
{
class IProxyObject;
class ICLEntry;

namespace MuCommands
{
	bool SetPresence (IProxyObject*, ICLEntry*, const QString&);

	bool SetDirectedPresence (IProxyObject*, ICLEntry*, const QString&);
}
}
}
