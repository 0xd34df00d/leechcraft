/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QMap>
#include "structures.h"

namespace LC
{
namespace CpuLoad
{
	class Backend : public QObject
	{
		Q_OBJECT
	public:
		Backend (QObject* = nullptr);

		virtual void Update () = 0;

		virtual int GetCpuCount () const = 0;
		virtual QMap<LoadPriority, LoadTypeInfo> GetLoads (int cpu) const = 0;
	signals:
		void cpuCountChanged (int);
	};
}
}
