/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QSortFilterProxyModel>

namespace LC::BitTorrent
{
	class TabViewProxyModel : public QSortFilterProxyModel
	{
		enum class StateFilterMode
		{
			All,
			Downloading,
			Seeding
		} StateFilter_ = StateFilterMode::All;
	public:
		using QSortFilterProxyModel::QSortFilterProxyModel;

		void SetStateFilterMode (int);
	protected:
		bool filterAcceptsRow (int, const QModelIndex&) const override;
	};
}
