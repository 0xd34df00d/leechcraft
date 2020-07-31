/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QSortFilterProxyModel>
#include <QQueue>

namespace LC
{
namespace Aggregator
{
	class JobHolderRepresentation : public QSortFilterProxyModel
	{
		QModelIndex Selected_;
	public:
		JobHolderRepresentation (QObject* = nullptr);

		QModelIndex SelectionChanged (const QModelIndex&);
	protected:
		bool filterAcceptsRow (int, const QModelIndex&) const override;
	};
}
}
