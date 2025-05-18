/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "checkableproxymodel.h"

namespace LC::Util
{
	CheckableProxyModelBase::CheckableProxyModelBase (int idRole, QObject *parent)
	: QIdentityProxyModel { parent }
	, IdRole_ { idRole }
	{
	}

	Qt::ItemFlags CheckableProxyModelBase::flags (const QModelIndex& index) const
	{
		auto flags = QIdentityProxyModel::flags (index);
		if (index.column () == 0)
			flags |= Qt::ItemIsUserCheckable;
		return flags;
	}

	QVariant CheckableProxyModelBase::data (const QModelIndex& index, int role) const
	{
		if (role == Qt::CheckStateRole && index.column () == 0)
			return IsChecked (index.data (IdRole_)) ? Qt::Checked : Qt::Unchecked;

		return QIdentityProxyModel::data (index, role);
	}

	bool CheckableProxyModelBase::setData (const QModelIndex& index, const QVariant& value, int role)
	{
		if (role == Qt::CheckStateRole && index.column () == 0)
		{
			SetChecked (index.data (IdRole_), value.value<Qt::CheckState> () == Qt::Checked);
			emit selectionChanged ();
			emit dataChanged (index, index, { Qt::CheckStateRole });
			return true;
		}

		return QIdentityProxyModel::setData (index, value, role);
	}

	void CheckableProxyModelBase::CheckAll ()
	{
		const auto rc = sourceModel ()->rowCount ();
		for (int i = 0; i < rc; ++i)
		{
			const auto srcIndex = sourceModel ()->index (i, 0);
			const auto& idVar = srcIndex.data (IdRole_);
			if (!IsChecked (idVar))
			{
				SetChecked (idVar, true);
				const auto& index = mapFromSource (srcIndex);
				emit dataChanged (index, index, { Qt::CheckStateRole });
			}
		}

		emit selectionChanged ();
	}

	void CheckableProxyModelBase::CheckNone ()
	{
		const auto rc = sourceModel ()->rowCount ();
		for (int i = 0; i < rc; ++i)
		{
			const auto srcIndex = sourceModel ()->index (i, 0);
			const auto& idVar = srcIndex.data (IdRole_);
			if (IsChecked (idVar))
			{
				SetChecked (idVar, false);
				const auto& index = mapFromSource (srcIndex);
				emit dataChanged (index, index, { Qt::CheckStateRole });
			}
		}

		emit selectionChanged ();
	}
}
