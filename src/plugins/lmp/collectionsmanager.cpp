/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "collectionsmanager.h"
#include <QStandardItemModel>
#include <QtDebug>
#include <util/models/mergemodel.h>
#include "interfaces/lmp/icollectionmodel.h"
#include "collectionsortermodel.h"
#include "player.h"

namespace LC
{
namespace LMP
{
	CollectionsManager::CollectionsManager (QObject *parent)
	: QObject { parent }
	, Model_ { new Util::MergeModel { { {} }, this } }
	, Sorter_ { new CollectionSorterModel { this } }
	{
		Sorter_->setSourceModel (Model_);
		Sorter_->setDynamicSortFilter (true);
		Sorter_->sort (0);
	}

	void CollectionsManager::Add (QAbstractItemModel *model)
	{
		Model_->AddModel (model);
	}

	QAbstractItemModel* CollectionsManager::GetModel () const
	{
		return Sorter_;
	}

	void CollectionsManager::Enqueue (const QList<QModelIndex>& indexes, Player *player)
	{
		QList<AudioSource> sources;
		for (const auto& idx : indexes)
		{
			auto srcIdx = Model_->mapToSource (Sorter_->mapToSource (idx));
			if (auto proxyModel = qobject_cast<const QSortFilterProxyModel*> (srcIdx.model ()))
				srcIdx = proxyModel->mapToSource (srcIdx);

			const auto& urls = dynamic_cast<const ICollectionModel*> (srcIdx.model ())->ToSourceUrls ({ srcIdx });
			for (const auto& url : urls)
				sources << url;
		}

		player->Enqueue (sources);
	}
}
}
