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

#ifndef PLUGINS_AZOTH_INTERFACES_IHAVESERVICEDISCOVERY_H
#define PLUGINS_AZOTH_INTERFACES_IHAVESERVICEDISCOVERY_H
#include <QMetaType>

class QAbstractItemModel;
class QString;

namespace LeechCraft
{
namespace Azoth
{
	class ISDSession
	{
	public:
		virtual ~ISDSession () {}
		
		virtual void SetQuery (const QString& query) = 0;
		virtual QAbstractItemModel* GetRepresentationModel () const = 0;
	};

	class IHaveServiceDiscovery
	{
	public:
		virtual ~IHaveServiceDiscovery () {}
		
		virtual QObject* CreateSDSession () = 0;
	};
}
}

Q_DECLARE_INTERFACE (LeechCraft::Azoth::ISDSession,
		"org.Deviant.LeechCraft.Azoth.ISDSession/1.0");
Q_DECLARE_INTERFACE (LeechCraft::Azoth::IHaveServiceDiscovery,
		"org.Deviant.LeechCraft.Azoth.IHaveServiceDiscovery/1.0");

#endif
