/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QSortFilterProxyModel>

class QMenu;
class QToolBar;

namespace LC::Aggregator
{
	class JobHolderRepresentation : public QSortFilterProxyModel
	{
		QModelIndex Selected_;
	public:
		struct Deps
		{
			QToolBar& Toolbar_;
			QWidget& DetailsWidget_;
			QMenu& RowMenu_;
			int SelectedRole_;
		};
	private:
		Deps Deps_;
	public:
		explicit JobHolderRepresentation (const Deps&, QObject* = nullptr);

		QVariant data (const QModelIndex&, int) const override;
	protected:
		bool filterAcceptsRow (int, const QModelIndex&) const override;
	};
}
