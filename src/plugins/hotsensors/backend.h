/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include "structures.h"

namespace LC
{
namespace HotSensors
{
	class Backend : public QObject
	{
		Q_OBJECT
	public:
		Backend (QObject* = nullptr);
	public slots:
		virtual void update () = 0;
	signals:
		void gotReadings (const Readings_t&);
	};
}
}
