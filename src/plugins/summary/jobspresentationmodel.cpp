/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "jobspresentationmodel.h"
#include <QStringList>
#include <interfaces/ijobholder.h>
#include <interfaces/structures.h>
#include <util/sll/visitor.h>
#include <util/util.h>
#include "util.h"

namespace LC::Summary
{
	void JobsPresentationModel::setSourceModel (QAbstractItemModel *model)
	{
		if (const auto oldModel = sourceModel ())
		{
			QSortFilterProxyModel::setSourceModel (nullptr);
			disconnect (oldModel, nullptr, this, nullptr);
		}

		if (!model)
			return;

		const auto handleReset = [this, model]
		{
			CachedRowInfos_.clear ();
			CachedRowInfos_.insert (0, model->rowCount (), {});
		};
		handleReset ();

		connect (model,
				&QAbstractItemModel::dataChanged,
				this,
				[this] (const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles)
				{
					const auto isInfoAffected = roles.isEmpty () || roles.contains (+JobHolderRole::RowInfo);
					if (!topLeft.parent ().isValid () && isInfoAffected)
						for (int i = topLeft.row (); i <= bottomRight.row (); ++i)
							CachedRowInfos_ [i].reset ();
				});

		connect (model,
				&QAbstractItemModel::rowsInserted,
				this,
				[this] (const QModelIndex& parent, int first, int last)
				{
					if (!parent.isValid ())
						CachedRowInfos_.insert (first, last - first + 1, {});
				});
		connect (model,
				&QAbstractItemModel::rowsAboutToBeRemoved,
				this,
				[this] (const QModelIndex& parent, int first, int last)
				{
					if (!parent.isValid ())
						CachedRowInfos_.remove (first, last - first + 1);
				});
		connect (model,
				&QAbstractItemModel::modelReset,
				this,
				handleReset);
		connect (model,
				&QAbstractItemModel::layoutChanged,
				this,
				handleReset);
		connect (model,
				&QAbstractItemModel::rowsMoved,
				this,
				[this] (const QModelIndex& parent, int first, int last,
						const QModelIndex& destination, int row)
				{
					const auto count = last - first + 1;
					if (!parent.isValid () && !destination.isValid ())
					{
						CachedRowInfos_.insert (row, count, {});
						if (row < first)
						{
							first += count;
							last += count;
						}
						const auto firstIt = CachedRowInfos_.begin () + first;
						const auto lastIt = CachedRowInfos_.begin () + last + 1;
						std::move (firstIt, lastIt, CachedRowInfos_.begin () + row);
						CachedRowInfos_.erase (firstIt, lastIt);
						return;
					}

					if (!parent.isValid ())
						CachedRowInfos_.remove (first, count);
					if (!destination.isValid ())
						CachedRowInfos_.insert (row, count, {});
				});

		QSortFilterProxyModel::setSourceModel (model);
	}

	int JobsPresentationModel::columnCount (const QModelIndex&) const
	{
		return ColumnCount;
	}

	QVariant JobsPresentationModel::data (const QModelIndex& index, int role) const
	{
		const auto& srcIdx = mapToSource (index.siblingAtColumn (0));
		switch (static_cast<JobHolderRole> (role))
		{
		case JobHolderRole::RowInfo:
			return QVariant::fromValue (GetRowInfo (srcIdx.row ()));
		}

		switch (static_cast<JobHolderProcessRole> (role))
		{
		case JobHolderProcessRole::Done:
		case JobHolderProcessRole::Total:
		case JobHolderProcessRole::ProgressCustomText:
		case JobHolderProcessRole::State:
		case JobHolderProcessRole::StateCustomText:
			return srcIdx.data (role);
		}

		switch (role)
		{
		case Qt::DisplayRole:
			return GetDisplayData (srcIdx, index.column ());
		case Qt::DecorationRole:
			return index.column () == Name ? srcIdx.data (Qt::DecorationRole) : QVariant {};
		case Qt::ToolTipRole:
		case Qt::ForegroundRole:
		case Qt::BackgroundRole:
		case Qt::FontRole:
		default:
			return srcIdx.data (role);
		}
	}

	Qt::ItemFlags JobsPresentationModel::flags (const QModelIndex& index) const
	{
		if (index.column () == 0)
			return mapToSource (index).flags ();
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	}

	QStringList JobsPresentationModel::GetTagsForIndex (int index) const
	{
		const auto model = sourceModel ();
		if (!model)
			return {};

		return model->index (index, 0).data (+CustomDataRoles::Tags).toStringList ();
	}

	namespace
	{
		QString GetStatusText (ProcessState state)
		{
			switch (state)
			{
			case ProcessState::Running:
				return JobsPresentationModel::tr ("running");
			case ProcessState::Paused:
				return JobsPresentationModel::tr ("paused");
			case ProcessState::Finished:
				return JobsPresentationModel::tr ("finished");
			case ProcessState::Error:
				return JobsPresentationModel::tr ("error");
			case ProcessState::Unknown:
				return {};
			}

			qWarning () << "unknown state" << static_cast<int> (state);
			return {};
		}
	}

	QVariant JobsPresentationModel::GetDisplayData (const QModelIndex& srcIdx, int column) const
	{
		if (srcIdx.row () >= CachedRowInfos_.size ())
		{
			qCritical () << "index outside of cached span!";
			return {};
		}

		const auto& rowInfo = GetRowInfo (srcIdx.row ());
		switch (column)
		{
		case Columns::Name:
			return rowInfo.Name_;
		case Columns::Status:
			return Util::Visit (rowInfo.Specific_,
					[&srcIdx] (const ProcessInfo&)
					{
						if (const auto& customText = srcIdx.data (+JobHolderProcessRole::StateCustomText).toString ();
							!customText.isNull ())
							return customText;

						const auto status = srcIdx.data (+JobHolderProcessRole::State).value<ProcessState> ();
						return GetStatusText (status);
					},
					[] (const NewsInfo& news) { return QString::number (news.Count_); });
		case Columns::Progress:
			return Util::Visit (rowInfo.Specific_,
					[&srcIdx] (const ProcessInfo& proc) -> QVariant
					{
						return MakeProgressString (proc,
								srcIdx.data (+JobHolderProcessRole::Done).toLongLong (),
								srcIdx.data (+JobHolderProcessRole::Total).toLongLong (),
								srcIdx);
					},
					[] (const NewsInfo& news) { return QVariant { news.LastUpdate_ }; });
		}

		return {};
	}

	const RowInfo& JobsPresentationModel::GetRowInfo (int row) const
	{
		auto& rowInfo = CachedRowInfos_ [row];
		if (!rowInfo)
			rowInfo = sourceModel ()->index (row, 0).data (+JobHolderRole::RowInfo).value<RowInfo> ();
		return *rowInfo;
	}
}
