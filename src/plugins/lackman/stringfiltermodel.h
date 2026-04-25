/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <util/models/fixedstringfilterproxymodel.h>

namespace LC::LackMan
{
	class StringFilterModel : public Util::FixedStringFilterProxyModel
	{
		QSet<QString> Tags_;
	public:
		explicit StringFilterModel (QObject* = nullptr);
	protected:
		void UpdateDerivedFilter (const QString&) override;
		bool filterAcceptsRow (int, const QModelIndex&) const override;
	};
}
