/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QtPlugin>

template<typename T>
class QList;

namespace LC
{
namespace Poshuku
{
	class IWebPlugin;

	class IWebPluginProvider
	{
	protected:
		virtual ~IWebPluginProvider () = default;
	public:
		virtual QList<IWebPlugin*> GetWebPlugins () = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::Poshuku::IWebPluginProvider,
	"org.LeechCraft.Poshuku.IWebPluginProvider/1.0")
