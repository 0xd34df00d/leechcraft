/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#ifndef PLUGINS_AZOTH_INTERFACES_IHAVESEARCH_H
#define PLUGINS_AZOTH_INTERFACES_IHAVESEARCH_H
#include <QMetaType>

class QAbstractItemModel;
class QModelIndex;
class QString;

namespace LeechCraft
{
namespace Azoth
{
	class ISearchSession
	{
	public:
		virtual ~ISearchSession () {}

		virtual void RestartSearch (QString server) = 0;

		virtual QAbstractItemModel* GetRepresentationModel () const = 0;
	};

	class IHaveSearch
	{
	public:
		virtual ~IHaveSearch () {}

		virtual QObject* CreateSearchSession () = 0;

		virtual QString GetDefaultSearchServer () const = 0;
	};
}
}

Q_DECLARE_INTERFACE (LeechCraft::Azoth::ISearchSession,
		"org.Deviant.LeechCraft.Azoth.ISearchSession/1.0");
Q_DECLARE_INTERFACE (LeechCraft::Azoth::IHaveSearch,
		"org.Deviant.LeechCraft.Azoth.IHaveSearch/1.0");

#endif
