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

namespace LC
{
namespace LMP
{
	class UploadModel : public QIdentityProxyModel
	{
		QSet<QPersistentModelIndex> SourceIndexes_;
	public:
		UploadModel (QObject* = 0);

		QSet<QPersistentModelIndex> GetSelectedIndexes () const;

		Qt::ItemFlags flags (const QModelIndex&) const;
		QVariant data (const QModelIndex&, int) const;
		bool setData (const QModelIndex&, const QVariant&, int);
	};
}
}
