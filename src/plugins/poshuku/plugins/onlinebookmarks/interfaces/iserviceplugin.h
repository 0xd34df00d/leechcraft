/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_ISERVICEPLUGIN_H
#define PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_ISERVICEPLUGIN_H

namespace LC
{
namespace Poshuku
{
namespace OnlineBookmarks
{
	class IServicePlugin
	{
	public:
		virtual ~IServicePlugin () {};

		virtual QObject* GetQObject () = 0;

		virtual QObject* GetBookmarksService () const = 0;
	};
}
}
}

Q_DECLARE_INTERFACE (LC::Poshuku::OnlineBookmarks::IServicePlugin,
		"org.Deviant.LeechCraft.Poshuku.OnlineBookmarks.IServicePlugin/1.0")

#endif // PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_ISERVICEPLUGIN_H
