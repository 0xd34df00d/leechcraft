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

#pragma once

#include <memory>
#include <QObject>
#include <QHash>
#include <interfaces/core/icoreproxy.h>

namespace LeechCraft
{
namespace Launchy
{
	class Item;
	typedef std::shared_ptr<Item> Item_ptr;

	class ItemsFinder : public QObject
	{
		Q_OBJECT

		ICoreProxy_ptr Proxy_;
		QHash<QString, QList<Item_ptr>> Items_;

		bool IsReady_;
	public:
		ItemsFinder (ICoreProxy_ptr, QObject* = 0);

		bool IsReady () const;

		QHash<QString, QList<Item_ptr>> GetItems () const;
		Item_ptr FindItem (const QString& permanentID) const;
	public slots:
		void update ();
	signals:
		void itemsListChanged ();
	};
}
}
