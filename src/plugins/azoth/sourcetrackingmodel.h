/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <util/models/mergemodel.h>
#include <interfaces/azoth/iresourceplugin.h>

namespace LC
{
namespace Azoth
{
	template<typename SrcType>
	class SourceTrackingModel : public Util::MergeModel
	{
		QHash<QString, SrcType*> Option2Source_;
		QHash<const QAbstractItemModel*, SrcType*> Model2Source_;
	public:
		using MergeModel::MergeModel;

		void AddSource (SrcType *src)
		{
			const auto model = src->GetOptionsModel ();
			Model2Source_ [model] = src;
			HandleItems (model, 0, model->rowCount (), true);
			AddModel (model);
		}

		QList<SrcType*> GetAllSources () const
		{
			return Model2Source_.values ();
		}

		SrcType* GetSourceForOption (const QString& opt) const
		{
			return Option2Source_.value (opt);
		}
	protected:
		void HandleRowsInserted (QAbstractItemModel *model, const QModelIndex& idx, int from, int to) override
		{
			HandleItems (model, from, to, true);
			MergeModel::HandleRowsInserted (model, idx, from, to);
		}

		void HandleRowsAboutToBeRemoved (QAbstractItemModel *model, const QModelIndex& idx, int from, int to) override
		{
			HandleItems (model, from, to, false);
			MergeModel::HandleRowsAboutToBeRemoved (model, idx, from, to);
		}
	private:
		void HandleItems (const QAbstractItemModel *model, int from, int to, bool add)
		{
			const auto src = Model2Source_ [model];
			for (int i = from; i <= to; ++i)
			{
				const QString& option = model->index (i, 0).data ().toString ();
				if (option.isEmpty ())
					continue;

				if (add)
					Option2Source_ [option] = src;
				else
					Option2Source_.remove (option);
			}
		}
	};
}
}
