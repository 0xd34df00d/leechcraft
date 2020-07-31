/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include "interfaces/blogique/ipluginproxy.h"

namespace LC
{
namespace Blogique
{
	class PluginProxy : public QObject
					, public IPluginProxy
	{
		Q_OBJECT
		Q_INTERFACES (LC::Blogique::IPluginProxy)
	public:
		PluginProxy (QObject* = 0);
	};
}
}
