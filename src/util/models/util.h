/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QModelIndex>

namespace LC::Util
{
	template<typename F>
	void EnumerateChildren (const QModelIndex& idx, bool includingRoot, F&& f)
	{
		if (includingRoot)
			f (idx);

		auto model = idx.model ();
		for (int i = 0; i < model->rowCount (idx); ++i)
			EnumerateChildren (model->index (i, 0, idx), true, f);
	}

}
