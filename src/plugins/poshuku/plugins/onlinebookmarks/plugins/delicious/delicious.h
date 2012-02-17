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

#ifndef PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_PLUGINS_DELICIOUS_DELICIOUS_H
#define PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_PLUGINS_DELICIOUS_DELICIOUS_H

#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>
#include <interfaces/iserviceplugin.h>

namespace LeechCraft
{
namespace Poshuku
{
namespace OnlineBookmarks
{
namespace Delicious
{

	class DeliciousService;

	class Plugin : public QObject
				, public IPlugin2
				, public IInfo
				, public IServicePlugin
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IPlugin2
				LeechCraft::Poshuku::OnlineBookmarks::IServicePlugin)

		std::shared_ptr<DeliciousService> DeliciousService_;
	public:
		void Init (ICoreProxy_ptr proxy);
		void SecondInit ();
		void Release ();
		QByteArray GetUniqueID() const;
		QString GetName() const;
		QString GetInfo() const;
		QIcon GetIcon() const;

		QSet<QByteArray> GetPluginClasses () const;

		QObject* GetObject ();
		QObject* GetBookmarksService () const;
	signals:
		void gotEntity (const LeechCraft::Entity&);
	};
}
}
}
}

#endif // PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_PLUGINS_DELICIOUS_DELICIOUS_H
