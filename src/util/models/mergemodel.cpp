/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include <algorithm>
#include <stdexcept>
#include <QMimeData>
#include <QUrl>
#include <QtDebug>
#include <util/sll/qtutil.h>
#include "mergemodel.h"

namespace LC::Util
{
	MergeModel::MergeModel (QStringList headers, QObject *parent)
	: QAbstractItemModel (parent)
	, Headers_ (std::move (headers))
	, Root_ (std::make_shared<ModelItem> ())
	{
	}

	int MergeModel::columnCount (const QModelIndex& index) const
	{
		if (!index.isValid ())
			return Headers_.size ();
		const auto& mapped = mapToSource (index);
		return mapped.model ()->columnCount (mapped);
	}

	QVariant MergeModel::headerData (int column, Qt::Orientation orient, int role) const
	{
		if (orient != Qt::Horizontal || role != Qt::DisplayRole)
			return QVariant ();

		return Headers_.at (column);
	}

	QVariant MergeModel::data (const QModelIndex& index, int role) const
	{
		if (!index.isValid ())
			return QVariant ();

		try
		{
			return mapToSource (index).data (role);
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
					<< e.what ();
			return {};
		}
	}

	Qt::ItemFlags MergeModel::flags (const QModelIndex& index) const
	{
		try
		{
			return mapToSource (index).flags ();
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
					<< e.what ();
			return {};
		}
	}

	QModelIndex MergeModel::index (int row, int column, const QModelIndex& parent) const
	{
		if (!hasIndex (row, column, parent))
			return {};

		auto parentItem = parent.isValid () ?
				static_cast<ModelItem*> (parent.internalPointer ()) :
				Root_.get ();

		return createIndex (row, column, parentItem->EnsureChild (row));
	}

	QModelIndex MergeModel::parent (const QModelIndex& index) const
	{
		if (!index.isValid () || index.internalPointer () == Root_.get ())
			return {};

		auto item = static_cast<ModelItem*> (index.internalPointer ());
		auto parent = item->GetParent ();
		if (parent == Root_)
			return {};

		return createIndex (parent->GetRow (), 0, parent.get ());
	}

	int MergeModel::rowCount (const QModelIndex& parent) const
	{
		if (!parent.isValid ())
			return Root_->GetRowCount ();

		const auto item = static_cast<ModelItem*> (parent.internalPointer ());
		return item->GetModel ()->rowCount (item->GetIndex ());
	}

	QStringList MergeModel::mimeTypes () const
	{
		QStringList result;
		for (const auto model : GetAllModels ())
			for (const auto& type : model->mimeTypes ())
				if (!result.contains (type))
					result << type;
		return result;
	}

	namespace
	{
		void Merge (QMimeData *out, const QMimeData *sub)
		{
			for (const auto& format : sub->formats ())
				if (format != "text/uri-list"_ql && !out->hasFormat (format))
					out->setData (format, sub->data (format));

			out->setUrls (out->urls () + sub->urls ());
		}
	}

	QMimeData* MergeModel::mimeData (const QModelIndexList& indexes) const
	{
		QMimeData *result = nullptr;

		for (const auto& index : indexes)
		{
			const auto& src = mapToSource (index);

			const auto subresult = src.model ()->mimeData ({ src });

			if (!subresult)
				continue;

			if (!result)
				result = subresult;
			else
			{
				Merge (result, subresult);
				delete subresult;
			}
		}

		return result;
	}

	QModelIndex MergeModel::mapFromSource (const QModelIndex& sourceIndex) const
	{
		if (!sourceIndex.isValid ())
			return {};

		QList<QModelIndex> hier;
		auto parent = sourceIndex;
		while (parent.isValid ())
		{
			hier.prepend (parent);
			parent = parent.parent ();
		}

		auto currentItem = Root_;
		for (const auto& idx : hier)
		{
			currentItem = currentItem->FindChild (idx);
			if (!currentItem)
			{
				qWarning () << Q_FUNC_INFO
						<< "no next item for"
						<< idx
						<< hier;
				return {};
			}
		}

		return createIndex (currentItem->GetRow (), sourceIndex.column (), currentItem.get ());
	}

	QModelIndex MergeModel::mapToSource (const QModelIndex& proxyIndex) const
	{
		const auto item = proxyIndex.isValid () ?
				static_cast<ModelItem*> (proxyIndex.internalPointer ()) :
				Root_.get ();

		const auto& srcIdx = item->GetIndex ();
		return srcIdx.sibling (srcIdx.row (), proxyIndex.column ());
	}

	void MergeModel::setSourceModel (QAbstractItemModel*)
	{
		throw std::runtime_error ("You should not set source model via setSourceModel()");
	}

	void MergeModel::SetHeaders (const QStringList& headers)
	{
		Headers_ = headers;
	}

	void MergeModel::AddModel (QAbstractItemModel *model)
	{
		if (!model)
			return;

		Models_.push_back (model);

		auto withModel = [this, model]<typename... Args> (void (MergeModel::*method) (QAbstractItemModel*, Args...))
		{
			return [this, model, method] (Args... args) { (this->*method) (model, args...); };
		};

		connect (model,
				&QAbstractItemModel::dataChanged,
				this,
				[this] (const QModelIndex& topLeft, const QModelIndex& bottomRight)
				{
					emit dataChanged (mapFromSource (topLeft), mapFromSource (bottomRight));
				});
		connect (model,
				&QAbstractItemModel::layoutAboutToBeChanged,
				this,
				withModel (&MergeModel::HandleModelAboutToBeReset));
		connect (model,
				&QAbstractItemModel::layoutChanged,
				this,
				withModel (&MergeModel::HandleModelReset));
		connect (model,
				&QAbstractItemModel::modelAboutToBeReset,
				this,
				withModel (&MergeModel::HandleModelAboutToBeReset));
		connect (model,
				&QAbstractItemModel::modelReset,
				this,
				withModel (&MergeModel::HandleModelReset));
		connect (model,
				&QAbstractItemModel::rowsAboutToBeInserted,
				this,
				withModel (&MergeModel::HandleRowsAboutToBeInserted));
		connect (model,
				&QAbstractItemModel::rowsAboutToBeRemoved,
				this,
				withModel (&MergeModel::HandleRowsAboutToBeRemoved));
		connect (model,
				&QAbstractItemModel::rowsInserted,
				this,
				withModel (&MergeModel::HandleRowsInserted));
		connect (model,
				&QAbstractItemModel::rowsRemoved,
				this,
				withModel (&MergeModel::HandleRowsRemoved));

		if (const auto rc = model->rowCount ())
		{
			beginInsertRows ({}, rowCount ({}), rowCount ({}) + rc - 1);

			for (auto i = 0; i < rc; ++i)
				Root_->AppendChild (model, model->index (i, 0), Root_);

			endInsertRows ();
		}
	}

	MergeModel::const_iterator MergeModel::FindModel (const QAbstractItemModel *model) const
	{
		return std::find (Models_.begin (), Models_.end (), model);
	}

	MergeModel::iterator MergeModel::FindModel (const QAbstractItemModel *model)
	{
		return std::find (Models_.begin (), Models_.end (), model);
	}

	void MergeModel::RemoveModel (QAbstractItemModel *model)
	{
		auto i = FindModel (model);

		if (i == Models_.end ())
		{
			qWarning () << Q_FUNC_INFO << "not found model" << model;
			return;
		}

		for (auto r = Root_->begin (); r != Root_->end (); )
			if ((*r)->GetModel () == model)
			{
				const auto idx = static_cast<int> (std::distance (Root_->begin (), r));

				beginRemoveRows ({}, idx, idx);
				r = Root_->EraseChild (r);
				endRemoveRows ();
			}
			else
				++r;

		Models_.erase (i);
	}

	size_t MergeModel::Size () const
	{
		return Models_.size ();
	}

	int MergeModel::GetStartingRow (MergeModel::const_iterator it) const
	{
		int result = 0;
		for (auto i = Models_.begin (); i != it; ++i)
			result += (*i)->rowCount ({});
		return result;
	}

	int MergeModel::GetStartingRow (MergeModel::iterator it)
	{
		int result = 0;
		for (auto i = Models_.begin (); i != it; ++i)
			result += (*i)->rowCount ({});
		return result;
	}

	MergeModel::const_iterator MergeModel::GetModelForRow (int row, int *starting) const
	{
		const auto child = Root_->GetChild (row);
		const auto it = FindModel (child->GetModel ());

		if (starting)
			*starting = GetStartingRow (it);

		return it;
	}

	MergeModel::iterator MergeModel::GetModelForRow (int row, int *starting)
	{
		const auto child = Root_->GetChild (row);
		const auto it = FindModel (child->GetModel ());

		if (starting)
			*starting = GetStartingRow (it);

		return it;
	}

	QList<QAbstractItemModel*> MergeModel::GetAllModels () const
	{
		QList<QAbstractItemModel*> result;
		for (auto p : Models_)
			if (p)
				result << p.data ();
		return result;
	}

	void MergeModel::HandleRowsAboutToBeInserted (QAbstractItemModel *model, const QModelIndex& parent, int first, int last)
	{
		const auto startingRow = parent.isValid () ?
				0 :
				GetStartingRow (FindModel (model));
		beginInsertRows (mapFromSource (parent),
				first + startingRow, last + startingRow);
	}

	void MergeModel::HandleRowsAboutToBeRemoved (QAbstractItemModel *model, const QModelIndex& parent, int first, int last)
	{
		const auto startingRow = parent.isValid () ?
				0 :
				GetStartingRow (FindModel (model));
		beginRemoveRows (mapFromSource (parent),
				first + startingRow, last + startingRow);

		const auto rawItem = parent.isValid () ?
				static_cast<ModelItem*> (mapFromSource (parent).internalPointer ()) :
				Root_.get ();
		const auto& item = rawItem->shared_from_this ();

		auto it = item->EraseChildren (item->begin () + startingRow + first,
				item->begin () + startingRow + last + 1);

		RemovalRefreshers_.push ([=] () mutable
				{
					for ( ; it != item->end () && (*it)->GetModel () == model; ++it)
						(*it)->RefreshIndex (startingRow);
				});
	}

	void MergeModel::HandleRowsInserted (QAbstractItemModel *model, const QModelIndex& parent, int first, int last)
	{
		const auto startingRow = parent.isValid () ?
				0 :
				GetStartingRow (FindModel (model));

		const auto rawItem = parent.isValid () ?
				static_cast<ModelItem*> (mapFromSource (parent).internalPointer ()) :
				Root_.get ();
		const auto& item = rawItem->shared_from_this ();

		for ( ; first <= last; ++first)
		{
			const auto& srcIdx = model->index (first, 0, parent);
			item->InsertChild (startingRow + first, model, srcIdx, item);
		}

		++last;
		last += startingRow;

		for (int rc = item->GetRowCount (); last < rc; ++last)
		{
			const auto child = item->GetChild (last);
			if (child->GetModel () != model)
				break;

			child->RefreshIndex (startingRow);
		}

		endInsertRows ();
	}

	void MergeModel::HandleRowsRemoved (QAbstractItemModel*, const QModelIndex&, int, int)
	{
		RemovalRefreshers_.pop () ();
		endRemoveRows ();
	}

	void MergeModel::HandleModelAboutToBeReset (QAbstractItemModel *model)
	{
		if (const auto rc = model->rowCount ())
		{
			const auto startingRow = GetStartingRow (FindModel (model));
			beginRemoveRows ({}, startingRow, rc + startingRow - 1);
			Root_->EraseChildren (Root_->begin () + startingRow, Root_->begin () + startingRow + rc);
			endRemoveRows ();
		}
	}

	void MergeModel::HandleModelReset (QAbstractItemModel *model)
	{
		if (const auto rc = model->rowCount ())
		{
			const auto startingRow = GetStartingRow (FindModel (model));

			beginInsertRows ({}, startingRow, rc + startingRow - 1);

			for (int i = 0; i < rc; ++i)
				Root_->InsertChild (startingRow + i, model, model->index (i, 0, {}), Root_);

			endInsertRows ();
		}
	}

	bool MergeModel::AcceptsRow (QAbstractItemModel*, int) const
	{
		DefaultAcceptsRowImpl_ = true;
		return true;
	}

	int MergeModel::RowCount (QAbstractItemModel *model) const
	{
		if (!model)
			return 0;

		int orig = model->rowCount ();
		if (DefaultAcceptsRowImpl_)
			return orig;

		int result = 0;
		for (int i = 0; i < orig; ++i)
			result += AcceptsRow (model, i) ? 1 : 0;
		return result;
	}
}
