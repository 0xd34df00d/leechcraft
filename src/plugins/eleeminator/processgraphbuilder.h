/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QAbstractItemModel>
#include "processinfo.h"

namespace LC::Eleeminator
{
	class ProcessGraphBuilder
	{
		const ProcessInfo Root_;
	public:
		ProcessGraphBuilder (int);

		ProcessInfo GetProcessTree () const;
		bool IsEmpty () const;

		QAbstractItemModel* CreateModel () const;
	};
}
