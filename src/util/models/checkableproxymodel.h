/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QIdentityProxyModel>
#include <QSet>
#include "modelsconfig.h"

namespace LC::Util
{
	template<typename IdType>
	class CheckableProxyModel : public QIdentityProxyModel
	{
		const int IdRole_;
		QSet<IdType> Unchecked_;
	public:
		explicit CheckableProxyModel (int idRole, QObject *parent = nullptr)
		: QIdentityProxyModel { parent }
		, IdRole_ { idRole }
		{
		}

		Qt::ItemFlags flags (const QModelIndex& index) const override
		{
			auto flags = QIdentityProxyModel::flags (index);
			if (index.column () == 0)
				flags |= Qt::ItemIsUserCheckable;
			return flags;
		}

		QVariant data (const QModelIndex& index, int role) const override
		{
			if (role == Qt::CheckStateRole && index.column () == 0)
			{
				const bool isChecked = !Unchecked_.contains (index.data (IdRole_).value<IdType> ());
				return isChecked ? Qt::Checked : Qt::Unchecked;
			}

			return QIdentityProxyModel::data (index, role);
		}

		bool setData (const QModelIndex& index, const QVariant& value, int role) override
		{
			if (role == Qt::CheckStateRole && index.column () == 0)
			{
				const auto id = index.data (IdRole_).value<IdType> ();
				if (value.value<Qt::CheckState> () == Qt::Checked)
					Unchecked_.remove (id);
				else
					Unchecked_ << id;

				emit dataChanged (index, index, { Qt::CheckStateRole });

				return true;
			}

			return QIdentityProxyModel::setData (index, value, role);
		}

		QSet<IdType> GetUnchecked () const
		{
			return Unchecked_;
		}

		QSet<IdType> GetChecked () const
		{
			const auto rc = sourceModel ()->rowCount ();

			QSet<IdType> result;
			result.reserve (rc - Unchecked_.size ());

			for (int i = 0; i < rc; ++i)
			{
				const auto rowId = sourceModel ()->index (i, 0).data (IdRole_).template value<IdType> ();
				if (!Unchecked_.contains (rowId))
					result << rowId;
			}

			return result;
		}

		void CheckAll ()
		{
			const auto unchecked = std::exchange (Unchecked_, {});

			const auto rc = sourceModel ()->rowCount ();
			for (int i = 0; i < rc; ++i)
			{
				const auto srcIndex = sourceModel ()->index (i, 0);
				const auto rowId = srcIndex.data (IdRole_).template value<IdType> ();
				if (unchecked.contains (rowId))
				{
					const auto& index = mapFromSource (srcIndex);
					emit dataChanged (index, index, { Qt::CheckStateRole });
				}
			}
		}

		void CheckNone ()
		{
			const auto rc = sourceModel ()->rowCount ();
			for (int i = 0; i < rc; ++i)
			{
				const auto srcIndex = sourceModel ()->index (i, 0);
				const auto rowId = srcIndex.data (IdRole_).template value<IdType> ();
				if (!Unchecked_.contains (rowId))
				{
					const auto& index = mapFromSource (srcIndex);
					Unchecked_ << rowId;
					emit dataChanged (index, index, { Qt::CheckStateRole });
				}
			}
		}
	};
}
