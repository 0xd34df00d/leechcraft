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

#include <QObject>
#include <interfaces/core/icoreproxy.h>

class QStandardItem;
class QAbstractItemModel;
class QStandardItemModel;

namespace LeechCraft
{
namespace Launchy
{
	class FavoritesManager;
	class ItemsFinder;
	class ItemImageProvider;

	class QuarkManager : public QObject
	{
		Q_OBJECT

		ICoreProxy_ptr Proxy_;
		FavoritesManager *FavMgr_;
		ItemsFinder *Finder_;
		ItemImageProvider *ImageProv_;

		QStandardItemModel *Model_;
	public:
		QuarkManager (ICoreProxy_ptr, FavoritesManager*, ItemsFinder*, ItemImageProvider*, QObject* = 0);

		QAbstractItemModel* GetModel () const;
	private:
		QStandardItem* MakeItem (const QString&) const;
	public slots:
		void launch (const QString&);
		void remove (const QString&);
	private slots:
		void init ();
		void addItem (const QString&);
		void handleItemRemoved (const QString&);
	};
}
}
