/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "modelitem.h"
#include <algorithm>

namespace LC
{
namespace Util
{
	ModelItem::ModelItem (QAbstractItemModel *model, const QModelIndex& idx, const ModelItem_wtr& parent)
	: ModelItemBase { parent }
	, Model_ { model }
	, SrcIdx_ { idx }
	{
	}

	ModelItem* ModelItem::EnsureChild (int row)
	{
		if (Children_.value (row))
			return Children_.at (row).get ();

		if (Children_.size () <= row)
			Children_.resize (row + 1);

		const auto& childIdx = Model_->index (row, 0, SrcIdx_);
		Children_ [row] = std::make_shared<ModelItem> (Model_, childIdx, shared_from_this ());
		return Children_.at (row).get ();
	}

	const QModelIndex& ModelItem::GetIndex () const
	{
		return SrcIdx_;
	}

	void ModelItem::RefreshIndex (int modelStartingRow)
	{
		if (SrcIdx_.isValid ())
			SrcIdx_ = Model_->index (GetRow () - modelStartingRow, 0, Parent_.lock ()->GetIndex ());
	}

	QAbstractItemModel* ModelItem::GetModel () const
	{
		return Model_;
	}

	ModelItem_ptr ModelItem::FindChild (QModelIndex index) const
	{
		index = index.sibling (index.row (), 0);

		const auto pos = std::find_if (Children_.begin (), Children_.end (),
				[&index] (const ModelItem_ptr& item) { return item->GetIndex () == index; });
		return pos == Children_.end () ? ModelItem_ptr {} : *pos;
	}
}
}
