/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#ifndef PLUGINS_NETSTOREMANAGER_INTERFACES_NETSTOREMANAGER_ISUPPORTFILELISTINGS_H
#define PLUGINS_NETSTOREMANAGER_INTERFACES_NETSTOREMANAGER_ISUPPORTFILELISTINGS_H
#include <QStringList>
#include <QtPlugin>

namespace LeechCraft
{
namespace NetStoreManager
{
	class ISupportFileListings
	{
	public:
		virtual ~ISupportFileListings () {}

		virtual void RefreshListing () = 0;
		virtual QStringList GetListingHeaders () const = 0;
	protected:
		virtual void gotListing (const QList<QStringList>&) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LeechCraft::NetStoreManager::ISupportFileListings,
		"org.Deviant.LeechCraft.NetStoreManager.ISupportFileListings/1.0");

#endif
