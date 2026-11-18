/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QList>
#include "backend.h"

namespace LC::CpuLoad
{
	class WinBackend final : public Backend
	{
		struct CpuTimes
		{
			qint64 Idle_;
			qint64 Kernel_;
			qint64 User_;
		};

		QList<CpuTimes> Prev_;
		QList<CpuTimes> Cur_;
	public:
		using Backend::Backend;

		void Update () override;

		int GetCpuCount () const override;
		QMap<LoadPriority, LoadTypeInfo> GetLoads (int cpu) const override;
	};
}
