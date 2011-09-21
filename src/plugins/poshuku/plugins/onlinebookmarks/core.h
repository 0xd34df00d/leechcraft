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

#ifndef PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_CORE_H
#define PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_CORE_H

#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/ibookmarksservice.h>

class QAbstractItemModel;
class QStandardItemModel;

namespace LeechCraft
{
namespace Poshuku
{
namespace OnlineBookmarks
{

	class PluginManager;

	class Core : public QObject
	{
		Q_OBJECT

		ICoreProxy_ptr CoreProxy_;
		QStandardItemModel *ActiveServicesModel_;
		boost::shared_ptr<PluginManager> PluginManager_;

		QObjectList ServicesPlugins_;

		Core ();
	public:
		static Core& Instance ();
		void SetProxy (ICoreProxy_ptr);
		ICoreProxy_ptr GetProxy () const;

		QAbstractItemModel* GetActiveServicesModel () const;

		QSet<QByteArray> GetExpectedPluginClasses () const;
		void AddPlugin (QObject*);

		QObjectList GetPlugins () const;

		QList<IBookmarksService*> GetBookmarksServices () const;
		
	};
}
}
}

#endif // PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_CORE_H
