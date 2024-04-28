/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QStandardItemModel>
#include <util/models/rolenamesmixin.h>

namespace LC::HotSensors
{
	class SensorsGraphModel : public Util::RoleNamesMixin<QStandardItemModel>
	{
	public:
		enum Role
		{
			LastTemp = Qt::UserRole + 1,
			SensorName,
			PointsList,
			MaxPointsList,
			MaxTemp,
			CritTemp,
			MaxPointsCount
		};

		explicit SensorsGraphModel (QObject*);
	};
}
