/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QSortFilterProxyModel>

namespace LC::LackMan
{
	class TypeFilterProxyModel : public QSortFilterProxyModel
	{
	public:
		enum class FilterMode
		{
			All,
			Installed,
			Upgradable,
			NotInstalled
		};
	private:
		FilterMode Mode_ = FilterMode::All;
	public:
		using QSortFilterProxyModel::QSortFilterProxyModel;

		void SetFilterMode (FilterMode);
	protected:
		bool filterAcceptsRow (int, const QModelIndex&) const override;
	};
}
