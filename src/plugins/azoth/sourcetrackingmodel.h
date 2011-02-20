/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#ifndef PLUGINS_AZOTH_SOURCETRACKINGMODEL_H
#define PLUGINS_AZOTH_SOURCETRACKINGMODEL_H
#include <plugininterface/mergemodel.h>
#include <interfaces/iresourceplugin.h>

namespace LeechCraft
{
namespace Azoth
{
	template<typename SrcType>
	class SourceTrackingModel : public Util::MergeModel
	{
		QHash<QString, SrcType*> Option2Source_;
		QHash<const QAbstractItemModel*, SrcType*> Model2Source_;
	public:
		SourceTrackingModel (const QStringList& strings, QObject *parent = 0)
		: MergeModel (strings, parent)
		{
		}

		void AddSource (SrcType *src)
		{
			QAbstractItemModel *model = src->GetOptionsModel ();
			Model2Source_ [model] = src;
			HandleItems (model, 0, model->rowCount (), true);
			AddModel (model);
		}

		SrcType* GetSourceForOption (const QString& opt) const
		{
			return Option2Source_.value (opt);
		}
	protected:
		virtual void handleRowsInserted (const QModelIndex& idx, int from, int to)
		{
			HandleItems (idx.model (), from, to, true);
			MergeModel::handleRowsInserted (idx, from, to);
		}

		virtual void handleRowsRemoved (const QModelIndex& idx, int from, int to)
		{
			HandleItems (idx.model (), from, to, false);
			MergeModel::handleRowsRemoved (idx, from, to);
		}
	private:
		void HandleItems (const QAbstractItemModel *model,
				int from, int to, bool add)
		{
			if (!model)
			{
				model = qobject_cast<QAbstractItemModel*> (sender ());
				
				const QAbstractProxyModel *proxy = 0;
				while ((proxy = qobject_cast<const QAbstractProxyModel*> (model)))
					model = proxy->sourceModel ();
			}

			SrcType *src = Model2Source_ [model];
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

#endif
