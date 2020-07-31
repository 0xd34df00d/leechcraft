/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QString>

namespace LC
{
namespace Blogique
{
	class IPluginProxy
	{
	public:
		virtual ~IPluginProxy () {}
	};
}
}

Q_DECLARE_INTERFACE (LC::Blogique::IPluginProxy,
		"org.Deviant.LeechCraft.Plugins.Blogique.Plugins.IPluginProxy/1.0")

