/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QHash>
#include "structures.h"

namespace LC::HotSensors
{
	class HistoryManager : public QObject
	{
		Q_OBJECT

		ReadingsHistory_t History_;
	public:
		using QObject::QObject;

		static int GetMaxHistorySize ();

		void HandleReadings (const Readings_t&);
	signals:
		void historyChanged (const ReadingsHistory_t&);
	};
}
