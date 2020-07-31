/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "backend.h"
#include <QVector>

namespace LC
{
namespace CpuLoad
{
	class MacBackend : public Backend
	{
		unsigned int CpuCount_ = 0;

		QVector<QMap<LoadPriority, double>> PrevLoads_,  Loads_;
	public:
		MacBackend (QObject* = nullptr);

		void Update ();

		int GetCpuCount () const;
		QMap<LoadPriority, LoadTypeInfo> GetLoads (int cpu) const;
	private:
		void UpdateCpuCount ();
		void UpdateLoads ();
	};
}
}
