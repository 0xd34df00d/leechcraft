/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#ifndef PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_ISERVICEPLUGIN_H
#define PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_ISERVICEPLUGIN_H

namespace LeechCraft
{
namespace Poshuku
{
namespace OnlineBookmarks
{
	class IServicePlugin
	{
	public:
		virtual ~IServicePlugin () {};

		virtual QObject* GetObject () = 0;

		virtual QObject* GetBookmarksService () const = 0;
	};
}
}
}

Q_DECLARE_INTERFACE (LeechCraft::Poshuku::OnlineBookmarks::IServicePlugin,
		"org.Deviant.LeechCraft.Poshuku.OnlineBookmarks.IServicePlugin/1.0");

#endif // PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_ISERVICEPLUGIN_H
