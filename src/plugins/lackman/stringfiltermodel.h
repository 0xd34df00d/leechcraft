/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_LACKMAN_STRINGFILTERMODEL_H
#define PLUGINS_LACKMAN_STRINGFILTERMODEL_H
#include <util/models/fixedstringfilterproxymodel.h>

namespace LC
{
namespace LackMan
{
	class StringFilterModel : public Util::FixedStringFilterProxyModel
	{
	public:
		StringFilterModel (QObject* = 0);
	protected:
		bool filterAcceptsRow (int, const QModelIndex&) const override;
	};
}
}

#endif
