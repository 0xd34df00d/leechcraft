/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QCoreApplication>
#include <util/tags/tagsfiltermodel.h>

namespace LC
{
	struct RowInfo;
}

namespace LC::Summary
{
	class JobsPresentationModel : public Util::TagsFilterModel
	{
		Q_DECLARE_TR_FUNCTIONS (LC::Summary::JobsPresentationModel)

		mutable QList<std::optional<RowInfo>> CachedRowInfos_;
	public:
		enum Columns
		{
			Name,
			Status,
			Progress,

			ColumnCount
		};

		void setSourceModel (QAbstractItemModel *sourceModel) override;

		int columnCount (const QModelIndex& parent) const override;
		QVariant data (const QModelIndex& index, int role) const override;
		Qt::ItemFlags flags (const QModelIndex& index) const override;
	protected:
		QStringList GetTagsForIndex (int) const override;
	private:
		QVariant GetDisplayData (const QModelIndex& srcIdx, int column) const;
		const RowInfo& GetRowInfo (int row) const;
	};
}
