/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013 Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <util/models/fixedstringfilterproxymodel.h>

namespace LC
{
namespace Blogique
{
	class TagsProxyModel : public Util::FixedStringFilterProxyModel
	{
		Q_OBJECT

		Q_PROPERTY (int count READ GetCount NOTIFY countChanged)
	public:
		using FixedStringFilterProxyModel::FixedStringFilterProxyModel;

		int GetCount () const;
		Q_INVOKABLE QString GetTagName (int index);

		void countUpdated ();
	protected:
		bool lessThan (const QModelIndex& left, const QModelIndex& right) const override;
	signals:
		void countChanged ();
	};
}
}
