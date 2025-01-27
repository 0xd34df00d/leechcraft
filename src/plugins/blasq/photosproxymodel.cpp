/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "photosproxymodel.h"
#include <QStringList>
#include <QUrl>
#include <QtDebug>
#include "interfaces/blasq/collection.h"
#include "interfaces/blasq/isupportdeletes.h"

namespace LC
{
namespace Blasq
{
	namespace
	{
		const int ItemsInCollage = 3;

		enum ExtendedRole
		{
			SupportsDeletes = CollectionRole::CollectionRoleMax,
			IsSelected
		};
	}

	PhotosProxyModel::PhotosProxyModel (QObject *parent)
	: NamedModel<QIdentityProxyModel> {
			parent,
			{
				{ ExtendedRole::SupportsDeletes, "supportsDeletes" },
				{ ExtendedRole::IsSelected, "isSelected" },
			}
		}
	{
	}

	QVariant PhotosProxyModel::data (const QModelIndex& index, int role) const
	{
		if (role == ExtendedRole::SupportsDeletes)
			return SupportsDeletes_;
		else if (role == ExtendedRole::IsSelected)
			return Selected_.contains (index.data (CollectionRole::ID).toString ());

		const auto& srcIdx = mapToSource (index);
		const auto& srcData = srcIdx.data (role);
		if (!srcData.isNull ())
			return srcData;

		if (role != CollectionRole::SmallThumb)
			return srcData;

		const auto type = srcIdx.data (CollectionRole::Type).toInt ();
		if (type == ItemType::Image)
			return srcData;

		QVariantList result;
		for (int i = 0; i < std::min (ItemsInCollage, sourceModel ()->rowCount (srcIdx)); ++i)
		{
			const auto& photoIdx = sourceModel ()->index (i, 0, srcIdx);
			const auto& url = photoIdx.data (CollectionRole::SmallThumb);
			result << url;
		}
		std::reverse (result.begin (), result.end ());
		return result;
	}

	void PhotosProxyModel::setSourceModel (QAbstractItemModel *model)
	{
		if (sourceModel ())
			disconnect (sourceModel (),
					SIGNAL (rowsInserted (QModelIndex, int, int)),
					this,
					SLOT (handleRowsInserted (QModelIndex, int, int)));

		QIdentityProxyModel::setSourceModel (model);

		if (model)
			connect (model,
					SIGNAL (rowsInserted (QModelIndex, int, int)),
					this,
					SLOT (handleRowsInserted (QModelIndex, int, int)));

	}

	void PhotosProxyModel::SetCurrentAccount (QObject *accObj)
	{
		SupportsDeletes_ = qobject_cast<ISupportDeletes*> (accObj);
	}

	void PhotosProxyModel::AddSelected (const QString& id, const QModelIndexList& idxs)
	{
		Selected_ << id;
		EmitDataChanged (idxs);
	}

	void PhotosProxyModel::RemoveSelected (const QString& id, const QModelIndexList& idxs)
	{
		Selected_.remove (id);
		EmitDataChanged (idxs);
	}

	void PhotosProxyModel::ClearSelected ()
	{
		Selected_.clear ();
	}

	void PhotosProxyModel::EmitDataChanged (const QModelIndexList& idxs)
	{
		for (const auto& idx : idxs)
		{
			const auto& mapped = mapFromSource (idx);
			emit dataChanged (mapped, mapped);
		}
	}

	void PhotosProxyModel::handleRowsInserted (const QModelIndex& parent, int from, int)
	{
		if (from >= ItemsInCollage)
			return;

		emit dataChanged (parent, parent);
	}
}
}
