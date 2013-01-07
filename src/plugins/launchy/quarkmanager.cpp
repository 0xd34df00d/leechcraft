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

#include "quarkmanager.h"
#include <QStandardItemModel>
#include "favoritesmanager.h"
#include "itemsfinder.h"
#include "item.h"
#include "itemimageprovider.h"

namespace LeechCraft
{
namespace Launchy
{
	namespace
	{
		class LaunchModel : public QStandardItemModel
		{
		public:
			enum Roles
			{
				PermanentID = Qt::UserRole + 1
			};

			LaunchModel (QObject *parent)
			: QStandardItemModel (parent)
			{
				QHash<int, QByteArray> roleNames;
				roleNames [Roles::PermanentID] = "permanentID";
				setRoleNames (roleNames);
			}
		};
	}

	QuarkManager::QuarkManager (ICoreProxy_ptr proxy, FavoritesManager *favMgr,
			ItemsFinder *finder, ItemImageProvider *prov, QObject *parent)
	: QObject (parent)
	, Proxy_ (proxy)
	, FavMgr_ (favMgr)
	, Finder_ (finder)
	, ImageProv_ (prov)
	, Model_ (new LaunchModel (this))
	{
		if (Finder_->IsReady ())
			init ();
		else
			connect (Finder_,
					SIGNAL (itemsListChanged ()),
					this,
					SLOT (init ()));

		connect (FavMgr_,
				SIGNAL (favoriteAdded (QString)),
				this,
				SLOT (addItem (QString)));
		connect (FavMgr_,
				SIGNAL (favoriteRemoved (QString)),
				this,
				SLOT (handleItemRemoved (QString)),
				Qt::QueuedConnection);
	}

	QAbstractItemModel* QuarkManager::GetModel () const
	{
		return Model_;
	}

	QStandardItem* QuarkManager::MakeItem (const QString& id) const
	{
		auto item = Finder_->FindItem (id);
		if (!item)
		{
			qWarning () << Q_FUNC_INFO
					<< "item not found"
					<< id;
			return 0;
		}

		ImageProv_->AddItem (item);

		auto modelItem = new QStandardItem;
		modelItem->setData (item->GetPermanentID (), LaunchModel::Roles::PermanentID);
		return modelItem;
	}

	void QuarkManager::launch (const QString& id)
	{
		auto item = Finder_->FindItem (id);
		if (!item)
		{
			qWarning () << Q_FUNC_INFO
					<< "item not found"
					<< id;
			return;
		}

		item->Execute (Proxy_);
	}

	void QuarkManager::remove (const QString& id)
	{
		FavMgr_->RemoveFavorite (id);
	}

	void QuarkManager::init ()
	{
		QList<QStandardItem*> items;
		for (const auto& id : FavMgr_->GetFavorites ())
			if (auto item = MakeItem (id))
				items << item;
		Model_->invisibleRootItem ()->appendRows (items);
	}

	void QuarkManager::addItem (const QString& id)
	{
		if (auto item = MakeItem (id))
			Model_->appendRow (item);
	}

	void QuarkManager::handleItemRemoved (const QString& id)
	{
		for (int i = 0, rc = Model_->rowCount (); i < rc; ++i)
			if (Model_->item (i)->data (LaunchModel::Roles::PermanentID) == id)
			{
				Model_->removeRow (i);
				break;
			}
	}
}
}
