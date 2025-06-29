/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QSortFilterProxyModel>
#include "modelsconfig.h"

namespace LC::Util
{
	class UTIL_MODELS_API FixedStringFilterProxyModel : public QSortFilterProxyModel
	{
	protected:
		QString FilterFixedString_;
	public:
		using QSortFilterProxyModel::QSortFilterProxyModel;

		void SetFilterString (const QString&);
	private:
		using QSortFilterProxyModel::setFilterFixedString;
	};
}
