/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "quarkmanager.h"
#include <QStandardItemModel>
#include <util/util.h>
#include <util/xdg/itemsfinder.h>
#include <util/xdg/item.h>
#include <util/models/rolenamesmixin.h>
#include "favoritesmanager.h"
#include "itemimageprovider.h"

namespace LC
{
namespace Launchy
{
	namespace
	{
		class LaunchModel : public Util::RoleNamesMixin<QStandardItemModel>
		{
		public:
			enum Roles
			{
				PermanentID = Qt::UserRole + 1,
				AppName
			};

			LaunchModel (QObject *parent)
			: RoleNamesMixin<QStandardItemModel> (parent)
			{
				QHash<int, QByteArray> roleNames;
				roleNames [Roles::PermanentID] = "permanentID";
				roleNames [Roles::AppName] = "appName";
				setRoleNames (roleNames);
			}
		};
	}

	QuarkManager::QuarkManager (ICoreProxy_ptr proxy, FavoritesManager *favMgr,
			Util::XDG::ItemsFinder *finder, ItemImageProvider *prov, QObject *parent)
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

		const auto& curLang = Util::GetLanguage ().toLower ();

		auto descr = item->GetComment (curLang);
		if (descr.isEmpty ())
			descr = item->GetGenericName (curLang);

		auto name = item->GetName (curLang);
		if (!descr.isEmpty ())
			name += " (" + descr + ")";
		modelItem->setData (name, LaunchModel::Roles::AppName);

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
		if (auto rc = Model_->rowCount ())
			Model_->removeRows (0, rc);

		QList<QStandardItem*> items;
		for (const auto& id : FavMgr_->GetFavorites ())
			if (auto item = MakeItem (id))
				items << item;

		if (!items.isEmpty ())
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
