/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "backend.h"

namespace LC
{
namespace CpuLoad
{
	class FreeBSDBackend final : public Backend
	{
	public:
		using Backend::Backend;

		void Update () override;

		int GetCpuCount () const override;
		QMap<LoadPriority, LoadTypeInfo> GetLoads (int cpu) const override;
	};
}
}
