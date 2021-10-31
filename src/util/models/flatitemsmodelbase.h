/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QAbstractItemModel>
#include "modelsconfig.h"

namespace LC::Util
{
	class UTIL_MODELS_API FlatItemsModelBase : public QAbstractItemModel
	{
		const QStringList Headers_;
	public:
		constexpr static auto DataRole = Qt::UserRole;

		explicit FlatItemsModelBase (QStringList headers, QObject* = nullptr);

		int columnCount (const QModelIndex& index = {}) const override;
		QVariant data (const QModelIndex& index, int role) const override;
		QVariant headerData (int section, Qt::Orientation orientation, int role) const override;
		QModelIndex index (int row, int col, const QModelIndex& parent = {}) const override;
		QModelIndex parent (const QModelIndex&) const override;
		int rowCount (const QModelIndex& parent = {}) const override;
	protected:
		virtual int GetItemsCount () const = 0;
		virtual QVariant GetData (int row, int col, int role) const = 0;
	};
}
