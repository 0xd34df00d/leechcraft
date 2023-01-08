/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "q2wproxymodel.h"
#include <iterator>
#include <QAbstractItemModel>
#include <QVector>
#include <QtDebug>
#include <QDateTime>
#include <QIcon>
#include <Wt/WDateTime.h>
#include <Wt/WApplication.h>
#include <util/util.h>
#include "util.h"

namespace LC
{
namespace Aggregator
{
namespace WebAccess
{
	Q2WProxyModel::Q2WProxyModel (QAbstractItemModel& src, Wt::WApplication *app)
	: Src_ { src }
	, Root_ { new Util::ModelItem { &src, {}, {} } }
	, App_ { app }
	, Update_ { app }
	{
		const auto type = Qt::DirectConnection;
		connect (&src,
				SIGNAL (dataChanged (QModelIndex, QModelIndex)),
				this,
				SLOT (handleDataChanged (QModelIndex, QModelIndex)),
				type);

		connect (&src,
				SIGNAL (rowsAboutToBeInserted (QModelIndex, int, int)),
				this,
				SLOT (handleRowsAboutToBeInserted (QModelIndex, int, int)),
				type);
		connect (&src,
				SIGNAL (rowsInserted (QModelIndex, int, int)),
				this,
				SLOT (handleRowsInserted (QModelIndex, int, int)),
				type);
		connect (&src,
				SIGNAL (rowsAboutToBeRemoved (QModelIndex, int, int)),
				this,
				SLOT (handleRowsAboutToBeRemoved (QModelIndex, int, int)),
				type);
		connect (&src,
				SIGNAL (rowsRemoved (QModelIndex, int, int)),
				this,
				SLOT (handleRowsRemoved (QModelIndex, int, int)),
				type);

		connect (&src,
				SIGNAL (modelAboutToBeReset ()),
				this,
				SLOT (handleModelAboutToBeReset ()),
				type);
		connect (&src,
				SIGNAL (modelReset ()),
				this,
				SLOT (handleModelReset ()),
				type);
	}

	void Q2WProxyModel::SetRoleMappings (const QMap<int, int>& mapping)
	{
		Mapping_ = mapping;
	}

	void Q2WProxyModel::AddDataMorphism (const Morphism_t& morphism)
	{
		Morphisms_ << morphism;
	}

	QModelIndex Q2WProxyModel::MapToSource (const Wt::WModelIndex& index) const
	{
		return W2QIdx (index);
	}

	int Q2WProxyModel::columnCount (const Wt::WModelIndex& parent) const
	{
		return Src_.columnCount (W2QIdx (parent));
	}

	int Q2WProxyModel::rowCount (const Wt::WModelIndex& parent) const
	{
		return Src_.rowCount (W2QIdx (parent));
	}

	Wt::WModelIndex Q2WProxyModel::parent (const Wt::WModelIndex& index) const
	{
		if (!index.isValid ())
			return {};

		if (!index.internalPointer () ||
				index.internalPointer () == Root_.get ())
			return {};

		const auto child = static_cast<Util::ModelItem*> (index.internalPointer ());
		const auto parentItem = child->GetParent ();
		if (parentItem == Root_)
			return {};

		return createIndex (parentItem->GetRow (), 0, parentItem.get ());
	}

	namespace
	{
		const int IconSize = 16;

		Wt::cpp17::any Variant2Any (const QVariant& var)
		{
			switch (var.type ())
			{
			case QVariant::Bool:
				return var.toBool ();
			case QVariant::DateTime:
				return Wt::WDateTime::fromTime_t (var.toDateTime ().toSecsSinceEpoch ());
			case QVariant::String:
				return ToW (var.toString ());
			case QVariant::Double:
				return var.toDouble ();
			case QVariant::Int:
				return var.toInt ();
			case QVariant::ULongLong:
				return var.toULongLong ();
			case QVariant::Icon:
			{
				const auto& icon = var.value<QIcon> ();
				if (icon.isNull ())
					return {};

				return ToW (Util::GetAsBase64Src (icon.pixmap (IconSize, IconSize).toImage ()));
			}
			default:
				if (var.canConvert<double> ())
					return var.toDouble ();
				if (var.canConvert<int> ())
					return var.toInt ();
				if (var.canConvert<QString> ())
					return ToW (var.toString ());
				return {};
			}
		}
	}

	Wt::cpp17::any Q2WProxyModel::data (const Wt::WModelIndex& index, Wt::ItemDataRole role) const
	{
		const auto& src = W2QIdx (index);

		for (const auto& m : Morphisms_)
		{
			const auto& result = m (src, role);
			if (!result.empty ())
				return result;
		}

		return Variant2Any (src.data (WtRole2Qt (role)));
	}

	Wt::WModelIndex Q2WProxyModel::index (int row, int column, const Wt::WModelIndex& parent) const
	{
		if (!hasIndex (row, column, parent))
			return {};

		const auto parentPtr = parent.internalPointer () ?
				static_cast<Util::ModelItem*> (parent.internalPointer ()) :
				Root_.get ();
		return createIndex (row, column, parentPtr->EnsureChild (row));
	}

	Wt::cpp17::any Q2WProxyModel::headerData (int section, Wt::Orientation orientation, Wt::ItemDataRole role) const
	{
		if (orientation != Wt::Orientation::Horizontal || role != Wt::ItemDataRole::Display)
			return Wt::WAbstractItemModel::headerData (section, orientation, role);

		return Variant2Any (Src_.headerData (section, Qt::Horizontal, Qt::DisplayRole));
	}

	void* Q2WProxyModel::toRawIndex (const Wt::WModelIndex& index) const
	{
		return index.internalPointer () ? index.internalPointer () : Root_.get ();
	}

	Wt::WModelIndex Q2WProxyModel::fromRawIndex (void *rawIndex) const
	{
		if (rawIndex == Root_.get ())
			return {};

		auto items = Root_->GetChildren ();
		for (int i = 0; i < items.size (); ++i)
		{
			auto thisItem = items.at (i);
			if (thisItem.get () == rawIndex)
				return createIndex (thisItem->GetRow (), 0, thisItem.get ());

			items << thisItem->GetChildren ();
		}

		return {};
	}

	QModelIndex Q2WProxyModel::W2QIdx (const Wt::WModelIndex& index) const
	{
		if (!index.isValid ())
			return {};

		const auto ptr = index.internalPointer () ?
				static_cast<Util::ModelItem*> (index.internalPointer ()) :
				Root_.get ();
		const auto& srcIdx = ptr->GetIndex ();
		return srcIdx.sibling (index.row (), index.column ());
	}

	Wt::WModelIndex Q2WProxyModel::Q2WIdx (const QModelIndex& index) const
	{
		if (!index.isValid ())
			return {};

		struct Info
		{
			QModelIndex Idx_;
		};
		QList<Info> parentsRows;

		auto parent = index;
		while (parent.isValid ())
		{
			parentsRows.prepend ({ parent });
			parent = parent.parent ();
		}
		parentsRows.removeFirst ();

		auto current = Root_;
		for (const auto& info : parentsRows)
			if (!(current = current->GetChild (info.Idx_.row ())))
				return {};

		return createIndex (index.row (), index.column (), current.get ());
	}

	int Q2WProxyModel::WtRole2Qt (Wt::ItemDataRole role) const
	{
		switch (role.value ())
		{
		case Wt::ItemDataRole::Display:
			return Qt::DisplayRole;
		case Wt::ItemDataRole::Decoration:
			return Qt::DecorationRole;
		}

		return Mapping_.value (role.value (), -1);
	}

	void Q2WProxyModel::handleDataChanged (const QModelIndex& topLeft, const QModelIndex& bottomRight)
	{
		const auto& wtl = Q2WIdx (topLeft);
		const auto& wbr = Q2WIdx (bottomRight);
		if (!wtl.isValid () || !wbr.isValid ())
			return;

		Wt::WApplication::UpdateLock lock { App_ };
		dataChanged () (wtl, wbr);

		Update_ ();
	}

	void Q2WProxyModel::handleRowsAboutToBeInserted (const QModelIndex& srcParent, int first, int last)
	{
		const auto& parent = Q2WIdx (srcParent);
		if (!parent.isValid ())
			return;

		rowsAboutToBeInserted () (parent, first, last);
	}

	void Q2WProxyModel::handleRowsInserted (const QModelIndex& srcParent, int first, int last)
	{
		const auto& parent = Q2WIdx (srcParent);
		if (!parent.isValid ())
			return;

		const auto rawItem = static_cast<Util::ModelItem*> (parent.internalPointer ());
		const auto item = rawItem->shared_from_this ();

		for ( ; first <= last; ++first)
			item->InsertChild (first, &Src_, Src_.index (first, 0, srcParent), item);

		for ( ; first < item->GetRowCount (); ++first)
			item->GetChild (first)->RefreshIndex (0);

		rowsInserted () (parent, first, last);
	}

	void Q2WProxyModel::handleRowsAboutToBeRemoved (const QModelIndex& srcParent, int first, int last)
	{
		const auto& parent = Q2WIdx (srcParent);
		if (!parent.isValid ())
			return;

		rowsAboutToBeRemoved () (parent, first, last);

		const auto rawItem = static_cast<Util::ModelItem*> (parent.internalPointer ());
		const auto item = rawItem->shared_from_this ();

		auto next = item->EraseChildren (item->begin () + first, item->begin () + last + 1);
		for ( ; next != item->end (); ++next)
			(*next)->RefreshIndex (0);
	}

	void Q2WProxyModel::handleRowsRemoved (const QModelIndex& srcParent, int first, int last)
	{
		const auto& parent = Q2WIdx (srcParent);
		if (!parent.isValid ())
			return;

		rowsRemoved () (parent, first, last);
	}

	void Q2WProxyModel::handleModelAboutToBeReset ()
	{
		Wt::WApplication::UpdateLock lock { App_ };

		if ((LastModelResetRC_ = rowCount ({})))
			rowsAboutToBeRemoved () ({}, 0, LastModelResetRC_ - 1);
	}

	void Q2WProxyModel::handleModelReset ()
	{
		Wt::WApplication::UpdateLock lock { App_ };

		if (LastModelResetRC_)
			rowsRemoved () ({}, 0, LastModelResetRC_ - 1);

		LastModelResetRC_ = 0;

		if (const auto rc = rowCount ({}))
		{
			rowsAboutToBeInserted () ({}, 0, rc - 1);
			rowsInserted () ({}, 0, rc - 1);
		}

		Update_ ();
	}
}
}
}
