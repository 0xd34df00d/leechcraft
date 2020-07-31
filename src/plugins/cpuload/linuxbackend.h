/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QString>
#include <QList>
#include <QVector>
#include "backend.h"

namespace LC
{
namespace CpuLoad
{
	typedef QVector<QVector<long>> Cummulative_t;

	class LinuxBackend final : public Backend
	{
		QVector<QMap<LoadPriority, LoadTypeInfo>> Loads_;

		Cummulative_t LastCummulative_;
	public:
		LinuxBackend (QObject* = nullptr);

		void Update ();

		int GetCpuCount () const;
		QMap<LoadPriority, LoadTypeInfo> GetLoads (int cpu) const;
	};
}
}
