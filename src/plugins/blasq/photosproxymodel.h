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
#include "interfaces/blasq/collection.h"

namespace LC
{
namespace Blasq
{
	class PhotosProxyModel : public NamedModel<QIdentityProxyModel>
	{
		Q_OBJECT

		bool SupportsDeletes_ = false;
		QSet<QString> Selected_;
	public:
		PhotosProxyModel (QObject* = 0);

		QVariant data (const QModelIndex&, int) const override;
		void setSourceModel (QAbstractItemModel *sourceModel) override;

		void SetCurrentAccount (QObject*);

		void AddSelected (const QString&, const QModelIndexList&);
		void RemoveSelected (const QString&, const QModelIndexList&);
		void ClearSelected ();
	private:
		void EmitDataChanged (const QModelIndexList&);
	private slots:
		void handleRowsInserted (const QModelIndex&, int, int);
	};
}
}
