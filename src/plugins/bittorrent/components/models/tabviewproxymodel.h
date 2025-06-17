/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <util/models/fixedstringfilterproxymodel.h>

namespace LC::BitTorrent
{
	class TabViewProxyModel : public Util::FixedStringFilterProxyModel
	{
		enum class StateFilterMode
		{
			All,
			Downloading,
			Seeding
		} StateFilter_ = StateFilterMode::All;
	public:
		using FixedStringFilterProxyModel::FixedStringFilterProxyModel;

		void SetStateFilterMode (int);
	protected:
		bool filterAcceptsRow (int, const QModelIndex&) const override;
	};
}
