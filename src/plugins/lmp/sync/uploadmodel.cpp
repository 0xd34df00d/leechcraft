/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "uploadmodel.h"
#include <QtDebug>

namespace LC
{
namespace LMP
{
	UploadModel::UploadModel (QObject *parent)
	: QIdentityProxyModel { parent }
	{
	}

	QSet<QPersistentModelIndex> UploadModel::GetSelectedIndexes () const
	{
		return SourceIndexes_;
	}

	Qt::ItemFlags UploadModel::flags (const QModelIndex& idx) const
	{
		return QIdentityProxyModel::flags (idx) | Qt::ItemIsUserCheckable;
	}

	QVariant UploadModel::data (const QModelIndex& idx, int role) const
	{
		const auto& var = QIdentityProxyModel::data (idx, role);
		if (role != Qt::CheckStateRole)
			return var;

		return SourceIndexes_.contains (mapToSource (idx)) ?
				Qt::Checked :
				Qt::Unchecked;
	}

	bool UploadModel::setData (const QModelIndex& idx, const QVariant& data, int role)
	{
		if (role != Qt::CheckStateRole)
			return false;

		if (data.toBool ())
		{
			SourceIndexes_ << mapToSource (idx);
			emit dataChanged (idx, idx);
		}
		else
		{
			auto parent = idx;
			while (parent.isValid ())
			{
				SourceIndexes_.remove (mapToSource (parent));
				emit dataChanged (parent, parent);
				parent = parent.parent ();
			}
		}

		for (int i = 0, rc = rowCount (idx); i < rc; ++i)
			setData (index (i, 0, idx), data, Qt::CheckStateRole);

		return true;
	}
}
}
