/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QCoreApplication>
#include <util/tags/categoryselector.h>

namespace LC::Aggregator
{
	class ItemCategorySelector : public Util::CategorySelector
	{
		Q_DECLARE_TR_FUNCTIONS (LC::Aggregator::ItemCategorySelector)

		bool CanBeVisible_ = true;
	public:
		explicit ItemCategorySelector (QWidget* = nullptr);

		void SetPossibleSelections (QStringList selections, bool sort = true) override;
	private:
		void UpdateVisibility ();
	};
}
