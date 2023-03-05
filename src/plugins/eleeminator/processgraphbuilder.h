/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include "processinfo.h"

class QAbstractItemModel;

namespace LC::Eleeminator
{
	class ProcessGraphBuilder
	{
		const ProcessInfo Root_;
	public:
		explicit ProcessGraphBuilder (int);

		ProcessInfo GetProcessTree () const;
		bool IsEmpty () const;

		std::unique_ptr<QAbstractItemModel> CreateModel () const;
	};
}
