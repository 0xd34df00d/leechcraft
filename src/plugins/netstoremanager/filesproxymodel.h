/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin <MaledictusDeMagog@gmail.com>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QSortFilterProxyModel>

namespace LC
{
namespace NetStoreManager
{
	class FilesProxyModel : public QSortFilterProxyModel
	{
		Q_OBJECT
	public:
		FilesProxyModel (QObject *parent = 0);
	protected:
		bool filterAcceptsRow (int sourceRow, const QModelIndex& sourceParent) const;
		bool lessThan (const QModelIndex& left, const QModelIndex& right) const;
	};
}
}
